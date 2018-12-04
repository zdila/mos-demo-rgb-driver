load('api_pwm.js');
load('api_mqtt.js');

MQTT.sub('sk.eastcode.demo/rgb', function (conn, topic, msg) {
  print(msg);
  let rgb = JSON.parse(msg);
  PWM.set(5, freq(rgb.r), duty(rgb.r));
  PWM.set(4, freq(rgb.g), duty(rgb.g));
  PWM.set(14, freq(rgb.b), duty(rgb.b));
});

function freq(v) {
  return duty(v) < 0.005 ? 0 : 400;
}

function duty(v) {
  return Math.pow(v / 255, 3);
}
