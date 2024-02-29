/*
Copyright 2019 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include "userosc.h"

static struct {
  float shape = 0.f;
  float shiftshape = 0.f;
} s_param;

static struct {
  float phi0 = 0.f;
  float w00 = 0.f;
} s_state;

void OSC_INIT(uint32_t platform, uint32_t api)
{
  return;
}

void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames)
{
  q31_t * __restrict y = (q31_t *)yn;
  const q31_t * y_e = y + frames;

  s_state.w00 = osc_w0f_for_note( (params->pitch)>>8, params->pitch & 0xFF );

  // Temporaries.
  float phi0 = s_state.phi0;
  const float w00 = s_state.w00;

  for (; y != y_e; ) {
    const float sig = 0.5f < phi0 ? 1.0f : -1.0f;
    *(y++) = f32_to_q31(sig);

    phi0 += w00;
    phi0 -= (uint32_t)phi0;
  }

  s_state.phi0 = phi0;
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
  return;
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
  return;
}

void OSC_PARAM(uint16_t index, uint16_t value)
{
  switch (index) {
  case k_user_osc_param_shape:
    s_param.shape = clip01f( param_val_to_f32(value) );
    break;
  case k_user_osc_param_shiftshape:
    s_param.shiftshape = clip01f( param_val_to_f32(value) );
    break;
  // case k_user_osc_param_id1:
  //   ... = (int32_t)value;
  //   break;
  // case k_user_osc_param_id2:
  //   ... = (int32_t)value;
  //   break;
  // case k_user_osc_param_id3:
  //   ... = (int32_t)value;
  //   break;
  default:
    break;
  }
}
