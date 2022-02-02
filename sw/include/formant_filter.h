#ifndef FORMANT_FILTER_H
#define	FORMANT_FILTER_H

#include <stdint.h>
#include "fp_lib_typeconv.h"

#define NOF_VOWELS_POW2 2

#define NOF_VOWELS ((1 << NOF_VOWELS_POW2) + 1)

#define NOF_FORMANT_FILTER_STAGES 4

static inline int16_t calcLinearInterpolation(
                                              const _Q15 y1,
                                              const _Q15 y2,
                                              const _Q15 x)
{
    _Q15 y = 0;
    __asm__ volatile(
            "\
        lac     %[Y1], #0, A                                                       ;AccA = X1[k] \n \
        msc     %[Y1] * %[X], A                                                  ;AccA -= X1[k] * Mix => AccA = X1[k] * (1-Mix) \n \
        mac     %[Y2] * %[X], A                                                  ;AccA += X2[k] * Mix => AccA = X1[k] * (1-Mix) + X2[k] * Mix \n \
        sac.r   A, #0, %[Y]                                                ;X2[k] = AccB \n \
                                                                                ;\n \
        ; 4 cycles total"
            : [Y]"+r"(y) /*out*/
            : [Y1]"z"(y1), [Y2]"z"(y2), [X]"z"(x) /*in*/
            : /*clobbered*/
            );

    return y;
}

static inline int16_t calcFreq(
                               const uint16_t stage,
                               const _Q16 shape)
{
    static const int16_t freqTable[NOF_VOWELS][NOF_FORMANT_FILTER_STAGES] = {
        {15647, 17236, 19944, 21248},
        {14021, 19096, 20065, 20854},
        {12321, 19498, 20656, 21248},
        {14014, 15787, 20025, 20757},
        {12610, 17328, 19772, 20886}
    };

    const uint16_t shapeInt = shape >> (16 - NOF_VOWELS_POW2);
    const _Q15 shapeFract = convert_Q16_Q15(shape << NOF_VOWELS_POW2);
    const _Q15 freq = calcLinearInterpolation(
                                              freqTable[shapeInt][stage],
                                              freqTable[shapeInt + 1][stage],
                                              shapeFract);

    return freq;
}

//

static inline _Q16 calcResonance(
                                 const uint16_t stage,
                                 const _Q16 shape)
{
    static const _Q15 resonanceTable[NOF_VOWELS][NOF_FORMANT_FILTER_STAGES] = {
        {30798, 31248, 31694, 31684},
        {29609, 31880, 31731, 31554},
        {27566, 31978, 31894, 31684},
        {29603, 30455, 31719, 31519},
        {27992, 31288, 31639, 31565}
    };

    const uint16_t shapeInt = shape >> (16 - NOF_VOWELS_POW2);
    const _Q15 shapeFract = convert_Q16_Q15(shape << NOF_VOWELS_POW2);
    const _Q15 resonance = calcLinearInterpolation(
                                                   resonanceTable[shapeInt][stage],
                                                   resonanceTable[shapeInt + 1][stage],
                                                   shapeFract);

    return convert_Q15_Q16_Naive(resonance);
}

#endif