load('api_pwm.js');
load('api_config.js');
load('api_rpc.js');

let pins = {
  r: Cfg.get('rgb.pin.r'),
  g: Cfg.get('rgb.pin.g'),
  b: Cfg.get('rgb.pin.b')
};

RPC.addHandler('setRGB', function(args) {
  print('RGB', JSON.stringify(args));

  if (
    typeof args === 'object' &&
    isBandValid(args.r) &&
    isBandValid(args.g) &&
    isBandValid(args.b)
  ) {
    PWM.set(pins.r, 1000, duty(args.r));
    PWM.set(pins.g, 1000, duty(args.g));
    PWM.set(pins.b, 1000, duty(args.b));

    return null;
  } else {
    print('Invalid parameters.');

    return {
      error: -32602,
      message: 'Bad request. Expected: {"r":<R>,"g":<G>,"b":<B>}'
    };
  }
});

function isBandValid(v) {
  return typeof v === 'number' && v >= 0 && v <= 1;
}

function duty(v) {
  return v * v * v;
}
