#include "mgos_app.h"
#include "mgos_mqtt.h"
#include "mgos_timers.h"
#include "mgos_pwm.h"
#include "frozen.h"
#include "common/cs_dbg.h"
#include <math.h>

static float cur_rgb[3];
static float dest_rgb[3];
static float velocity[3];

static int pins[3];

static mgos_timer_id tid = MGOS_INVALID_TIMER_ID;

static void write_cb(void *ud) {
  FILE *f = fopen("state.tmp", "w");

  if (f) {
    fwrite(dest_rgb, sizeof(dest_rgb[0]), 3, f);
    fclose(f);

    rename("state.tmp", "state");
  }
}

static void sub_cb(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud) {
  if (json_scanf(msg, msg_len, "{r:%f,g:%f,b:%f}", &dest_rgb[0], &dest_rgb[1], &dest_rgb[2]) == 3) {
    if (tid == MGOS_INVALID_TIMER_ID) {
      mgos_clear_timer(tid);
    }

    tid = mgos_set_timer(10000, 0, write_cb, NULL);
  }
}

static void render_frame_cb(void *ud) {
  for (int i = 0; i < 3; i++) {
    velocity[i] = (velocity[i] + 0.02 * (dest_rgb[i] - cur_rgb[i])) * 0.8;
    cur_rgb[i] = fmin(fmax(0.0, cur_rgb[i] + velocity[i]), 1.0);

    mgos_pwm_set(pins[i], 1000, cur_rgb[i] * cur_rgb[i] * cur_rgb[i]);
  }
}

enum mgos_app_init_result mgos_app_init(void) {
  pins[0] = mgos_sys_config_get_rgb_pin_r();
  pins[1] = mgos_sys_config_get_rgb_pin_g();
  pins[2] = mgos_sys_config_get_rgb_pin_b();

  struct mg_str topic = MG_NULL_STR;

  mg_asprintf((char **) &topic.p, 0, "%s/rgb", mgos_sys_config_get_device_id());

  mgos_mqtt_sub(topic.p, sub_cb, NULL);

  mgos_set_timer(20, MGOS_TIMER_REPEAT, render_frame_cb, NULL);

  FILE *f = fopen("state", "r");

  if (f) {
    fread(dest_rgb, sizeof(dest_rgb[0]), 3, f);
    fclose(f);
  }

  return MGOS_APP_INIT_SUCCESS;
}
