/*
Copyright 2019 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include "userdelfx.h"
#include "buffer_ops.h"
#include "LCWDelay.h"
#include "LCWDelayParam.h"

// memo:
// BPM=30, 四分音符の場合
// delaySamples = (samplingRate * delayTime * 60) / bpm
//              = (48000 * 1 * 60) / 30
//              = 96000
// 必要なバイト数は約375k（= 96000 x 4byte）なので512k（または1024k）用意する
//
// 0x40000 = 2^18 = 262144 = 5.46(sec) * 48000
// 262144 * sizeof(float) / 1024 = 1024K
#define LCW_DELAY_SAMPLING_SIZE (0x40000)
static __sdram float s_delay_ram_sampling[LCW_DELAY_SAMPLING_SIZE];

static float s_time;
static float s_depth;
static float s_mix;
static float s_inputGain;

static LCWDelayBuffer delayLine;
static LCWDelayBlock delayBlock;

// 0〜1を0..(n-1)にマッピング
static inline int32_t f32_to_index(float val, int32_t n)
{
    return clipmaxi32( (int32_t)(val * n), (n - 1) );
}

void DELFX_INIT(uint32_t platform, uint32_t api)
{
    delayLine.buffer = &s_delay_ram_sampling[0];
    delayLine.size = LCW_DELAY_SAMPLING_SIZE;
    delayLine.mask = LCW_DELAY_SAMPLING_SIZE - 1;
    delayLine.pointer = 0;

    delayBlock.delayLine = &delayLine;
    delayBlock.param.fbGain = 0.f;
    delayBlock.param.position = 0.f;
    delayBlock.currentPosition = 0.f;

    s_time = 0.f;
    s_depth = 0.f;
    s_mix = 0.5f;
    s_inputGain = 0.f;
}

#define LCW_DELAY_SAMPLING_RATE (48000)
void DELFX_PROCESS(float *xn, uint32_t frames)
{
    float bpm = fx_get_bpmf();
    bpm = clampfsel(30.f, bpm, 480.f);
    const int32_t timeIndex = f32_to_index(s_time, LCW_DELAY_TIME_PARAMS);
    const float delayTime = delayTimeParams[timeIndex];
    // bps = bpm / 60
    // n = bps / delayTime
    // delaySamples = samplingRate / n
    //              = (samplingRate * delayTime) / bps
    //              = (samplingRate * delayTime * 60) / bpm
    delayBlock.param.position = (LCW_DELAY_SAMPLING_RATE * delayTime * 60) / bpm;

    const int32_t depthIndex = f32_to_index(s_depth, LCW_DELAY_FB_GAIN_TABLE_SIZE);
    delayBlock.param.fbGain = -1 * delayFbGainTable[depthIndex]; // memo: 符号反転はお好み

    float *__restrict x = xn;
    const float *x_e = x + 2 * frames;

    const float dry = 1.f - s_mix;
    const float wet = s_mix;

    // 切り替え時のノイズ対策
    if (s_inputGain < 0.99998f) {
        for (; x != x_e;) {
            *(x++) = *x * s_inputGain;
            *(x++) = *x * s_inputGain;

            if (s_inputGain < 0.99998f) {
                s_inputGain += ((1.f - s_inputGain) * 0.0625f);
            }
            else {
                s_inputGain = 1.f;
                break;
            }
        }
    }

    x = xn;
    for (; x != x_e;) {
        const float xL = *(x + 0);
        // const float xR = *(x + 1);

        const float wL = LCWDelayProcess(&delayBlock, xL);
        // const float wR = xR;

        *(x++) = (dry * xL) + (wet * wL);
        *(x++) = (dry * xL) + (wet * wL);
        // *(x++) = (dry * xR) + (wet * wR);
    }
}

void DELFX_RESUME(void)
{
    s_inputGain = 0.f;
}

void DELFX_PARAM(uint8_t index, int32_t value)
{
    const float valf = q31_to_f32(value);
    switch (index) {
    case k_user_delfx_param_time:
        s_time = clip01f(valf);
        break;
    case k_user_delfx_param_depth:
        s_depth = clip01f(valf);
        break;
    case k_user_delfx_param_shift_depth:
        // Rescale to add notch around 0.5f
        s_mix = (valf <= 0.49f) ? 1.02040816326530612244f * valf : (valf >= 0.51f) ? 0.5f + 1.02f * (valf - 0.51f)
                                                                                   : 0.5f;
        break;
    default:
        break;
    }
}
