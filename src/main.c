#include "mgos.h"
#include "mgos_app.h"
#include "mgos_rpc.h"
#include "mgos_pwm.h"
#include "mgos_config.h"
#include "mgos_time.h"
#include "paj7620.h"
#include <math.h>

void HSVToRGB(double h, double s, double v, double *pr, double *pg, double *pb) {
	double r = 0, g = 0, b = 0;

	if (s == 0) {
		r = v;
		g = v;
		b = v;
	} else {
		int i;
		double f, p, q, t;

    h *= 180 / M_PI;
  	h = h == 360 ? 0 : h / 60;

		i = (int) trunc(h);
		f = h - i;

		p = v * (1.0 - s);
		q = v * (1.0 - (s * f));
		t = v * (1.0 - (s * (1.0 - f)));

		switch (i) {
      case 0:
        r = v;
        g = t;
        b = p;
        break;

      case 1:
        r = q;
        g = v;
        b = p;
        break;

      case 2:
        r = p;
        g = v;
        b = t;
        break;

      case 3:
        r = p;
        g = q;
        b = v;
        break;

      case 4:
        r = t;
        g = p;
        b = v;
        break;

      default:
        r = v;
        g = p;
        b = q;
        break;
		}
	}

  *pr = r;
	*pg = g;
	*pb = b;
}

static double duty(double v) {
  return v * v * v;
}

static int freq(double v) {
  return duty(v) < 0.005 ? 0 : 400;
}

double h = 0, s = 1, v = 1;

int dH = 0;
int dV = 0;

static void apply() {
  const struct mgos_config_rgb_pin *pins = mgos_sys_config_get_rgb_pin();

  double r, g, b;
  HSVToRGB(h, s, v, &r, &g, &b);

  mgos_pwm_set(pins->r, freq(r), duty(r));
  mgos_pwm_set(pins->g, freq(g), duty(g));
  mgos_pwm_set(pins->b, freq(b), duty(b));
}

static void set_hsv_cb(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args) {
  if (json_scanf(args.p, args.len, ri->args_fmt, &h, &s, &v) == 3) {
    apply();
    mg_rpc_send_responsef(ri, "null");
  } else {
    mg_rpc_send_errorf(ri, -1, "Bad request. Expected: {\"h\":R,\"s\":G,\"v\":B}");
  }
  (void) cb_arg;
  (void) fi;
}

static void timer_cb(void *user_data) {
  (void) user_data;
  uint8_t data;

  if (dH || dV) {
    h += dH / 150.0;
    while (h < 0) {
      h += 2 * M_PI;
    }
    while (h > 2 * M_PI) {
      h -= 2 * M_PI;
    }

    v += dV / 1000.0;
    if (v < 0) {
      v = 0;
      dV = 0;
    }
    if (v > 1) {
      v = 1;
      dV = 0;
    }

    apply();
  }

	// Read Bank_0_Reg_0x43/0x44 for gesture result.
	if (paj7620ReadReg(0x43, 1, &data)) {
    return;
  }

  switch (data) { // When different gestures be detected, the variable 'data' will be set to different values by paj7620ReadReg(0x43, 1, &data).
    case GES_RIGHT_FLAG:
      if (dH > -1) {
        dH -= 1;
      }
      LOG(LL_INFO, ("Right"));
      break;
    case GES_LEFT_FLAG:
      if (dH < 1) {
        dH += 1;
      }
      LOG(LL_INFO, ("Left"));
      break;
    case GES_UP_FLAG:
      if (dV < 1) {
        dV += 1;
      }
      LOG(LL_INFO, ("Up"));
      break;
    case GES_DOWN_FLAG:
      if (dV > -1) {
        dV -= 1;
      }
      LOG(LL_INFO, ("Down"));
      break;
    case GES_FORWARD_FLAG:
      LOG(LL_INFO, ("Forward"));
      break;
    case GES_BACKWARD_FLAG:
      LOG(LL_INFO, ("Backward"));
      break;
    case GES_CLOCKWISE_FLAG:
      LOG(LL_INFO, ("Clockwise"));
      break;
    case GES_COUNT_CLOCKWISE_FLAG:
      LOG(LL_INFO, ("anti-clockwise"));
      break;
    default:
      paj7620ReadReg(0x44, 1, &data);
      if (data == GES_WAVE_FLAG) {
        LOG(LL_INFO, ("wave"));
      }
      break;
  }
}

enum mgos_app_init_result mgos_app_init(void) {
  mg_rpc_add_handler(mgos_rpc_get_global(), "setHSV", "{ h: %lf, s: %lf, v: %lf }", set_hsv_cb, NULL);

	LOG(LL_INFO, ("PAJ7620U2 TEST DEMO: Recognize 9 gestures."));

	uint8_t error = paj7620Init();			// initialize Paj7620 registers
	if (error) {
		LOG(LL_INFO, ("INIT ERROR,CODE: %d", error));
	} else {
		LOG(LL_INFO, ("INIT OK"));
	}

  mgos_set_timer(10, true, timer_cb, NULL);

  apply();

  return MGOS_APP_INIT_SUCCESS;
}
