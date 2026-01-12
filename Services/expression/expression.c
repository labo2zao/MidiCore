#include "Services/expression/expression.h"
#include "Services/router/router.h"
#include <string.h>
#include <math.h>

static expr_cfg_t g_cfg;
static uint16_t g_raw = 0;
static int32_t g_pa = 0;
static uint32_t g_ms = 0;
static float g_filt = 0.0f;
static uint8_t g_last_sent = 255;
static int8_t g_last_dir = 0;

static inline uint8_t clamp8(int v){ 
  if(v<0) return 0; 
  if(v>127) return 127; 
  return (uint8_t)v; 
}

static float apply_curve(float t){
  if(t < 0.0f) t = 0.0f;
  if(t > 1.0f) t = 1.0f;

  switch(g_cfg.curve){
    default:
    case EXPR_CURVE_LINEAR: return t;
    case EXPR_CURVE_EXPO: {
      float gamma = (g_cfg.curve_param > 0) ? ((float)g_cfg.curve_param / 100.0f) : 1.8f;
      if(gamma < 0.2f) gamma = 0.2f;
      if(gamma > 5.0f) gamma = 5.0f;
      return powf(t, gamma);
    }
    case EXPR_CURVE_S:
      return t*t*(3.0f - 2.0f*t);
  }
}

static uint8_t map_linear(uint16_t r){
  if(g_cfg.raw_max == g_cfg.raw_min) return g_cfg.out_min;
  int32_t x = (int32_t)r - (int32_t)g_cfg.raw_min;
  int32_t den = (int32_t)g_cfg.raw_max - (int32_t)g_cfg.raw_min;
  float t = (float)x / (float)den;
  t = apply_curve(t);
  float y = (float)g_cfg.out_min + t * (float)((int)g_cfg.out_max - (int)g_cfg.out_min);
  return clamp8((int)(y + 0.5f));
}

static uint8_t map_bidir(uint8_t is_push){
  // g_raw is expected centered mapping 0..4095 (pressure.ngc MAP_MODE=1)
  float t = 0.0f;
  if(is_push){
    int32_t r = (int32_t)g_raw - 2048;
    if(r < 0) r = 0;
    t = (float)r / 2047.0f;
  } else {
    int32_t r = 2048 - (int32_t)g_raw;
    if(r < 0) r = 0;
    t = (float)r / 2048.0f;
  }
  t = apply_curve(t);
  float y = (float)g_cfg.out_min + t * (float)((int)g_cfg.out_max - (int)g_cfg.out_min);
  return clamp8((int)(y + 0.5f));
}

static int should_send(uint8_t out){
  if(g_last_sent == 255) return 1;
  int delta = (int)out - (int)g_last_sent;
  int ad = delta < 0 ? -delta : delta;
  int8_t dir = (delta > 0) ? +1 : (delta < 0 ? -1 : 0);
  if(dir == 0) return 0;

  uint8_t thr = g_cfg.deadband_cc ? g_cfg.deadband_cc : 1;
  if(g_last_dir != 0 && dir != g_last_dir){
    thr = (uint8_t)(thr + g_cfg.hyst_cc);
  }
  return (ad >= thr);
}

static void send_cc(uint8_t cc, uint8_t val){
  router_msg_t m;
  m.type = ROUTER_MSG_3B;
  m.b0 = (uint8_t)(0xB0 | (g_cfg.midi_ch & 0x0F));
  m.b1 = cc & 0x7F;
  m.b2 = val;
  router_process(ROUTER_NODE_KEYS, &m);
}

void expression_init(void){
  memset(&g_cfg,0,sizeof(g_cfg));
  g_cfg.enable = 0;
  g_cfg.midi_ch = 0;
  g_cfg.cc_num = 11;
  g_cfg.cc_push = 11;
  g_cfg.cc_pull = 2;
  g_cfg.bidir = EXPR_BIDIR_OFF;
  g_cfg.raw_min = 0;
  g_cfg.raw_max = 4095;
  g_cfg.zero_deadband_pa = 500;
  g_cfg.out_min = 0;
  g_cfg.out_max = 127;
  g_cfg.rate_ms = 20;
  g_cfg.smoothing = 200;
  g_cfg.deadband_cc = 2;
  g_cfg.hyst_cc = 1;
  g_cfg.curve = EXPR_CURVE_EXPO;
  g_cfg.curve_param = 180;

  g_raw = 0;
  g_pa = 0;
  g_ms = 0;
  g_filt = 0.0f;
  g_last_sent = 255;
  g_last_dir = 0;
}

void expression_set_cfg(const expr_cfg_t* cfg){ if(cfg) g_cfg = *cfg; }
const expr_cfg_t* expression_get_cfg(void){ return &g_cfg; }

void expression_set_raw(uint16_t raw){ g_raw = raw; }
void expression_set_pressure_pa(int32_t pa){ g_pa = pa; }

void expression_tick_1ms(void){
  if(!g_cfg.enable){ g_ms=0; return; }
  g_ms++;

  uint8_t target;
  uint8_t cc = g_cfg.cc_num;

  if(g_cfg.bidir == EXPR_BIDIR_PUSH_PULL){
    // Neutral zone around 0 Pa
    if(g_pa < (int32_t)g_cfg.zero_deadband_pa && g_pa > -(int32_t)g_cfg.zero_deadband_pa){
      // stay near last value without flipping; small movements will be filtered by deadband/hysteresis
    }
    if(g_pa >= 0){
      cc = g_cfg.cc_push;
      target = map_bidir(1);
    } else {
      cc = g_cfg.cc_pull;
      target = map_bidir(0);
    }
  } else {
    target = map_linear(g_raw);
  }

  // EMA smoothing
  float a = 1.0f - ((float)g_cfg.smoothing / 255.0f);
  if(a < 0.02f) a = 0.02f;
  if(a > 1.0f) a = 1.0f;
  g_filt = g_filt + a * ((float)target - g_filt);
  uint8_t out = clamp8((int)(g_filt + 0.5f));

  if(g_ms >= (uint32_t)g_cfg.rate_ms){
    g_ms = 0;
    if(should_send(out)){
      int delta = (g_last_sent==255)?0:((int)out-(int)g_last_sent);
      int8_t dir = (delta > 0) ? +1 : (delta < 0 ? -1 : 0);
      if(dir != 0) g_last_dir = dir;
      g_last_sent = out;
      send_cc(cc, out);
    }
  }
}

void expression_runtime_reset(void){
  g_ms = 0;
  g_filt = 0.0f;
  g_last_sent = 255;
  g_last_dir = 0;
}
