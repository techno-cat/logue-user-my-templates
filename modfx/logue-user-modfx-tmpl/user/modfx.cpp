/*
Copyright 2019 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include "usermodfx.h"
#include "buffer_ops.h"

// static __sdram int32_t s_delay_ram_input[LCW_DELAY_INPUT_SIZE];
// static __sdram int32_t s_delay_ram_sampling[LCW_DELAY_SAMPLING_SIZE];

static float s_time;
static float s_depth;
static float s_inputGain;

void MODFX_INIT(uint32_t platform, uint32_t api)
{
  s_time = 0.f;
  s_depth = 0.f;
  s_inputGain = 0.f;
}

void MODFX_PROCESS(const float *main_xn, float *main_yn,
                   const float *sub_xn,  float *sub_yn,
                   uint32_t frames)
{
  const float * mx = main_xn;
  float * __restrict my = main_yn;
  const float * my_e = my + 2*frames;
  const float * sx = sub_xn;
  float * __restrict sy = sub_yn;

  for (; my != my_e; ) {
    const float xL = *(mx++) * s_inputGain;
    const float xR = *(mx++) * s_inputGain;

    // TODO: processing
    const float yL = xL;
    const float yR = xR;

    *(my++) = yL;
    *(my++) = yR;
    *(sy++) = yL;
    *(sy++) = yR;

    if ( s_inputGain < 0.99998f ) {
      s_inputGain += ( (1.f - s_inputGain) * 0.0625f );
    }
    else { s_inputGain = 1.f; }
  }
}

void MODFX_RESUME(void)
{
  s_inputGain = 0.f;
}

void MODFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) {
  case k_user_modfx_param_time:
    s_time = clip01f(valf);
    break;
  case k_user_modfx_param_depth:
    s_depth = clip01f(valf);
    break;
  default:
    break;
  }
}
