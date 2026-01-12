#include "ainser_profile_accordion_v1.h"
#include "ainser_map.h"

void ainser_profile_accordion_v1_apply(void) {
  AINSER_MapEntry *t = ainser_map_get_table();

  // Bellows primary (idx 16)
  t[16].channel   = 0;
  t[16].cc        = 20u + 16u;
  t[16].curve     = AINSER_CURVE_LOG;
  t[16].invert    = 1;
  t[16].min       = 200;
  t[16].max       = 3800;
  t[16].threshold = 12;

  // Bellows secondary (idx 17)
  t[17].channel   = 0;
  t[17].cc        = 20u + 17u;
  t[17].curve     = AINSER_CURVE_LOG;
  t[17].invert    = 0;
  t[17].min       = 200;
  t[17].max       = 3800;
  t[17].threshold = 12;

  // FX sends (idx 24..31) on MIDI ch3 (2)
  for (uint8_t ch = 0; ch < 8; ++ch) {
    uint8_t idx = (uint8_t)(24u + ch);
    t[idx].channel = 2; // MIDI ch3
  }

  // LH registers (idx 32..39) on MIDI ch2 (1)
  for (uint8_t ch = 0; ch < 8; ++ch) {
    uint8_t idx = (uint8_t)(32u + ch);
    t[idx].channel = 1; // MIDI ch2
  }

  // Dream SAM5716 (idx 48..55) on MIDI ch4 (3)
  t[48].channel   = 3; t[48].curve = AINSER_CURVE_LINEAR; t[48].threshold = 24; // SampleSelect
  t[49].channel   = 3; t[49].curve = AINSER_CURVE_EXPO;   t[49].threshold = 16; // LoopStart
  t[50].channel   = 3; t[50].curve = AINSER_CURVE_EXPO;   t[50].threshold = 16; // LoopEnd
  t[51].channel   = 3; t[51].curve = AINSER_CURVE_LINEAR;                      // Envelope
  t[52].channel   = 3; t[52].curve = AINSER_CURVE_LINEAR;                      // ReverbSend
  t[53].channel   = 3; t[53].curve = AINSER_CURVE_LINEAR;                      // ChorusSend
  t[54].channel   = 3; t[54].curve = AINSER_CURVE_EXPO;   t[54].threshold = 10; // FilterCutoff
  t[55].channel   = 3; t[55].curve = AINSER_CURVE_LINEAR;                       // FilterResonance
}