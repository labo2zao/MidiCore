#include "App/ain_midi_task.h"
#include "cmsis_os2.h"
#include "Services/ain/ain.h"
#include "Services/router/router.h"
#include "Services/midi/midi_delayq.h"
#include "Services/instrument/instrument_cfg.h"
#include "Services/zones/zones_cfg.h"
#include "Services/velocity/velocity.h"
#include "Services/humanize/humanize.h"
#include "Services/ui/ui.h"
#include "Services/input/input.h"
#include <string.h>

static uint8_t chord_cond_active(uint8_t velocity) {
  const instrument_cfg_t* c = instrument_cfg_get();
  if (!c->chord_cond_enable) return 1; // no condition => allow
  if (c->chord_vel_gt && !(velocity > c->chord_vel_gt)) return 0;
  if (c->chord_vel_lt && !(velocity < c->chord_vel_lt)) return 0;
  if (c->chord_need_hold) {
    if (!input_get_phys_state(c->hold_phys_id)) return 0;
  }
  if (c->chord_block_shift) {
    if (input_shift_active()) return 0;
  }
  return 1;
}

static uint16_t strum_delay(uint8_t i, uint8_t n, const instrument_cfg_t* c) {
  if (!c->strum_enable || n <= 1) return 0;
  uint8_t spread = c->strum_spread_ms;
  if (!spread) return 0;
  // distribute across [0..spread]
  if (n == 1) return 0;
  uint16_t step = (uint16_t)(spread / (n-1));
  return (uint16_t)(i * step);
}

  midi_delayq_send(ROUTER_NODE_KEYS, &m, (uint16_t)d);
}

static void send_note_ch(uint8_t ch, uint8_t note, uint8_t on, uint8_t vel,
                         uint16_t delay_ms, uint8_t apply_flag) {
  const instrument_cfg_t* c = instrument_cfg_get();
  router_msg_t m;
  m.type = ROUTER_MSG_3B;
  m.b0 = (uint8_t)((on ? 0x90 : 0x80) | (ch & 0x0F));
  m.b1 = note;
  m.b2 = on ? vel : 0;

  int8_t tJ = humanize_time_ms(c, apply_flag);
  int16_t d = (int16_t)delay_ms + (int16_t)tJ;
  if (d < 0) d = 0;
  if (d > 1000) d = 1000;

  if (on) {
    int16_t v = (int16_t)vel + (int16_t)humanize_vel_delta(c, apply_flag);
    if (v < 1) v = 1;
    if (v > 127) v = 127;
    m.b2 = (uint8_t)v;
  }
  midi_delayq_send(ROUTER_NODE_KEYS, &m, (uint16_t)d);
}

static void AinMidiTask(void* argument) {
  (void)argument;
  for (;;) {
    ain_event_t e;
    while (ain_pop_event(&e)) {
      if (e.type == AIN_EV_NOTE_ON) {
        const instrument_cfg_t* c = instrument_cfg_get();
        uint8_t vel = velocity_apply_curve(e.velocity, c);

        uint8_t chord_ui = ui_get_chord_mode();
        uint8_t chord_on = (chord_ui && chord_cond_active(vel));

        if (!chord_on) {
          send_note(e.key, 1, vel, 0, HUMAN_APPLY_KEYS);
        } else {
          uint8_t notes[4]; uint8_t preset=0;
          uint8_t n = chord_bank_expand(ui_get_chord_bank(), e.key, notes, &preset);
          // order for strum
          uint8_t order[4]={0,1,2,3};
          if (c->strum_dir == STRUM_DOWN && n>1) {
            for (uint8_t i=0;i<n/2;i++){ uint8_t t=order[i]; order[i]=order[n-1-i]; order[n-1-i]=t; }
          } else if (c->strum_dir == STRUM_RANDOM && n>1) {
            // Fisher-Yates (simple)
            for (int i=n-1;i>0;i--) { uint32_t r = (uint32_t)osKernelGetTickCount(); int j = (int)(r % (uint32_t)(i+1)); uint8_t t=order[i]; order[i]=order[j]; order[j]=t; }
          }
          for (uint8_t k=0;k<n;k++) {
            uint8_t i = order[k];
            uint8_t v2 = chord_preset_scale_vel(&ui_get_chord_bank()->preset[preset], i, vel);
            uint16_t d = strum_delay(k, n, c);
            send_note(notes[i], 1, v2, d, HUMAN_APPLY_CHORD);
          }
        }
      } else if (e.type == AIN_EV_NOTE_OFF) {
        // NOTE_OFF: if chord mode, release all chord notes same way (no vel)
        const instrument_cfg_t* c = instrument_cfg_get();
        uint8_t chord_ui = ui_get_chord_mode();
        if (!chord_ui) {
          send_note(e.key, 0, 0, 0, HUMAN_APPLY_KEYS);
        } else {
          uint8_t notes[4]; uint8_t preset=0;
          uint8_t n = chord_bank_expand(ui_get_chord_bank(), e.key, notes, &preset);
          for (uint8_t i=0;i<n;i++) send_note(notes[i], 0, 0, 0, HUMAN_APPLY_CHORD);
        }
      }
    }
    osDelay(1);
  }
}

void app_start_ain_midi_task(void) {
  const osThreadAttr_t attr = {
    .name="AinMIDI",
    .priority=osPriorityAboveNormal,
    .stack_size=1024
  };
  (void)osThreadNew(AinMidiTask, NULL, &attr);
}
