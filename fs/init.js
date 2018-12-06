load('api_pwm.js');
load('api_config.js');
load('api_rpc.js');

let pins = {
  r: Cfg.get('rgb.pin.r'),
  g: Cfg.get('rgb.pin.g'),
  b: Cfg.get('rgb.pin.b')
};

RPC.addHandler('setRGB', function (args) {
  print("RGB", JSON.stringify(args));
  if (
    typeof args === 'object'
    && isValidBand(args.r)
    && isValidBand(args.g)
    && isValidBand(args.b)
  ) {
    PWM.set(pins.r, freq(args.r), duty(args.r));
    PWM.set(pins.g, freq(args.g), duty(args.g));
    PWM.set(pins.b, freq(args.b), duty(args.b));

    return null;
  } else {
    print("Invalid parameters.");
    return { error: -32602, message: 'Bad request. Expected: {"r":<R>,"g":<G>,"b":<B>}' };
  }
});

function isValidBand(v) {
  return typeof v === 'number' && v >= 0 && v <= 1;
}

function freq(v) {
  return duty(v) < 0.005 ? 0 : 400;
}

function duty(v) {
  return v * v * v;
}
