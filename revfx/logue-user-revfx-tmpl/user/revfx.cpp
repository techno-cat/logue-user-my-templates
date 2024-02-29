/*
Copyright 2019 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include "userrevfx.h"
#include "buffer_ops.h"

// static __sdram int32_t s_reverb_ram_pre_buffer[LCW_REVERB_PRE_SIZE];
// static __sdram int32_t s_reverb_ram_comb_buffer[LCW_REVERB_COMB_BUFFER_TOTAL];
// static __sdram int32_t s_reverb_ram_ap_buffer[LCW_REVERB_AP_BUFFER_TOTAL];

static float s_time;
static float s_depth;
static float s_mix;
static float s_inputGain;

void REVFX_INIT(uint32_t platform, uint32_t api)
{
  s_time = 0.f;
  s_depth = 0.f;
  s_mix = 0.5f;
  s_inputGain = 0.f;
}

void REVFX_PROCESS(float *xn, uint32_t frames)
{
  float * __restrict x = xn;
  const float * x_e = x + 2*frames;

  const float dry = 1.f - s_mix;
  const float wet = s_mix;

  for (; x != x_e; ) {
    const float xL = *(x + 0) * s_inputGain;
    const float xR = *(x + 1) * s_inputGain;

    // TODO: processing
    const float wL = xL;
    const float wR = xR;

    *(x++) = (dry * xL) + (wet * wL);
    *(x++) = (dry * xR) + (wet * wR);

    if ( s_inputGain < 0.99998f ) {
      s_inputGain += ( (1.f - s_inputGain) * 0.0625f );
    }
    else { s_inputGain = 1.f; }
  }
}

void REVFX_RESUME(void)
{
  s_inputGain = 0.f;
}

void REVFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) {
  case k_user_revfx_param_time:
    s_time = clip01f(valf);
    break;
  case k_user_revfx_param_depth:
    s_depth = clip01f(valf);
    break;
  case k_user_revfx_param_shift_depth:
    // Rescale to add notch around 0.5f
    s_mix = (valf <= 0.49f) ? 1.02040816326530612244f * valf : (valf >= 0.51f) ? 0.5f + 1.02f * (valf-0.51f) : 0.5f;
    break;
  default:
    break;
  }
}
