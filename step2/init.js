load('api_pwm.js');
load('api_config.js');
load('api_rpc.js');
load('api_timer.js');

let pins = {
  r: Cfg.get('rgb.pin.r'),
  g: Cfg.get('rgb.pin.g'),
  b: Cfg.get('rgb.pin.b')
};

let start = {
  r: 0,
  g: 0,
  b: 0
};

let dest = {
  r: 0,
  g: 0,
  b: 0
};

let current = {
  r: 0,
  g: 0,
  b: 0
};

let ts = 0;

RPC.addHandler('setRGB', function (args) {
  print("RGB", JSON.stringify(args));
  if (
    typeof args === 'object'
    && isValidBand(args.r)
    && isValidBand(args.g)
    && isValidBand(args.b)
  ) {
    copyRgb(start, current);
    copyRgb(dest, args);
    ts = Timer.now();
    return null;
  } else {
    print("Invalid parameters.");
    return { error: -32602, message: 'Bad request. Expected: {"r":<R>,"g":<G>,"b":<B>}' };
  }
});

function copyRgb(d, s) {
  d.r = s.r;
  d.g = s.g;
  d.b = s.b;
}

Timer.set(20, Timer.REPEAT, function () {
  if (!ts) {
    return;
  }

  let d = Timer.now() - ts;
  if (d > 1) {
    d = 1;
    ts = 0;
  }

  current.r = start.r * (1 - d) + dest.r * d;
  current.g = start.g * (1 - d) + dest.g * d;
  current.b = start.b * (1 - d) + dest.b * d;

  PWM.set(pins.r, freq(current.r), duty(current.r));
  PWM.set(pins.g, freq(current.g), duty(current.g));
  PWM.set(pins.b, freq(current.b), duty(current.b));
}, undefined);

function isValidBand(v) {
  return typeof v === 'number' && v >= 0 && v <= 1;
}

function freq(v) {
  return duty(v) < 0.005 ? 0 : 400;
}

function duty(v) {
  return v * v * v;
}
