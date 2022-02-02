/*
Copyright (C) 2022 Andreas Lagler

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file osc_stacked_saw.h
 * @brief Implementation of stacked saw oscillator
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef OSC_STACKEDSAW_H
#define	OSC_STACKEDSAW_H

#include "osc_stacked_saw_types.h"
#include "SVF_2Pole.h"
#include "fp_lib_types.h"
#include "fp_lib_def.h"
#include <stdint.h>

/**
 * @brief Calculate parameters for stacked saw oscillator
 * 
 * Parameter calculation algorithms are explained here:\
 * Adam Szabo: How to Emulate the Super Saw\n
 * https://pdfs.semanticscholar.org/1852/250068e864215dd7f12755cf00636868a251.pdf\n
 * 
 * For design of 4th-order Butterworth filter see:\n
 * https://www.earlevel.com/main/2016/09/29/cascading-filters/
 * @param note Note on MIDI scale given in half-cents
 * @param freq Normalized oscillator frequency in Q0.32 format
 * @param shape1 The shape1 parameter in Q0.16 format translates into a side-oscillator detune
 * @param shape2 The shape2 parameter in Q0.16 format translates into a center/side-oscillator mixing ratio
 * @param params Struct holding stacked saw oscillator parameters
 */
inline static void calcOscStackedSawParams(
                                           const int16_t note,
                                           const _Q32 freq,
                                           const _Q16 shape1,
                                           const _Q16 shape2,
                                           OscStackedSawParams * const params)
{
    // Calculate weighted detune
    static const _Q16 offset[] = {0, 819, 1638, 2458, 3277, 4096, 4915, 5734, 6554, 9421, 12288, 15155, 18022, 20890, 23757, 26624};
    static const _Q16 multiplier[] = {819, 819, 819, 819, 819, 819, 819, 819, 2867, 2867, 2867, 2867, 2867, 2867, 2867, 38912};
    const _Q16 detuneWeighted = offset[shape1 >> 12] + mul_Q16_Q16(multiplier[shape1 >> 12], shape1 << 4);

    // Frequency of center oscillator
    const _Q32 freq2 = freq << 1; // Frequency times 2

    // Frequency of side oscillators
    const _Q16 detune1 = mul_Q16_Q16(646, detuneWeighted);
    params->freq[0] = mul_Q32_Q16(freq2, Q16_HALF + detune1);
    params->freq[1] = mul_Q32_Q16(freq2, Q16_HALF - detune1);

    const _Q16 detune2 = mul_Q16_Q16(2048, detuneWeighted);
    params->freq[2] = mul_Q32_Q16(freq2, Q16_HALF + detune2);
    params->freq[3] = mul_Q32_Q16(freq2, Q16_HALF - detune2);

    const _Q16 detune3 = mul_Q16_Q16(3604, detuneWeighted);
    params->freq[4] = mul_Q32_Q16(freq2, Q16_HALF + detune3);
    params->freq[5] = mul_Q32_Q16(freq2, Q16_HALF - detune3);

    // Center oscillator level = 1 - 0.55366 * Mix
    // We use lac + msc on Acc register to get "1 -"
    params->levelCenter = mul_Q15_Q16(18142, shape2);

    // Side oscillator level = -0.73764 * Mix^2 + 1.2841 * Mix + 0.044372
    _Q15 levelSide = mul_Q15_Q16(-12085, shape2);
    levelSide += 21039;
    levelSide = mul_Q15_Q16(levelSide, shape2);
    levelSide += 1454;
    params->levelSide = levelSide << 1;

    // Calculate filter coefficients for 4-pole Butterworth high pass
    calcCoeffs(
               note,
               4989,
               params->filterCoeffs1);

    calcCoeffs(
               note,
               40456,
               params->filterCoeffs2);
}

/**
 * @brief Calculate one sample of stacked saw oscillator waveform
 * 
 * Update the oscillator state and calculate one waveform sample from the updated state 
 * @param params Struct holding stacked saw oscillator parameters
 * @param state Struct holding stacked saw oscillator state
 * @return Stacked saw oscillator waveform sample in Q0.15 format
 */
inline static _Q15 calcOscStackedSaw(
                                     const OscStackedSawParams * const params,
                                     OscStackedSawState * const state)
{
    // Calculate weighted sum of 7 detuned oscillators
    _Q15 output;

    // Cache pointers for use with inline assembly
    _Q32 * phase = state->phase;
    const _Q32 * phaseInc = params->freq;
    const _Q15 levelCenter = params->levelCenter;
    const _Q15 levelSide = params->levelSide;

    __asm__ volatile(
            "\
    ;Calculate scaled center oscillator value in Accumulator A                  ;\n \
        mov     [%[phase]++], w4                                                ;Fetch Phase LSB into w4, phase has already been updated in the sync part of the oscillator \n \
        mov     [%[phase]++], w4                                                ;Fetch Phase MSB into w4, phase has already been updated in the sync part of the oscillator \n \
        lac     w4, #0, A                                                       ;Load center oscillator value into AccA \n \
        msc     w4 * %[levelCenter], A                                          ;AccA -= center oscillator value * center oscillator level (Now the total level is correct) \n \
                                                                                ;\n \
    ;Increment side oscillator 1 phase                                          ;\n \
        mov     [%[phase]], w4                                                  ;Fetch Phase into w4:w4 \n \
        add     w4, [%[phaseInc]++], [%[phase]++]                               ;Add LSBs \n \
        mov     [%[phase]], w4                                                  ;Fetch Phase into w4:w4 \n \
        addc    w4, [%[phaseInc]++], [%[phase]++]                               ;Add MSBs \n \
    ;Add scaled side oscillator 1 to Accumulator A                              ;\n \
        mac     w4 * %[levelSide], A                                            ;AccA += side oscillator value * side oscillator level \n \
                                                                                ;\n \
    ;Increment side oscillator 2 phase                                          ;\n \
        mov     [%[phase]], w4                                                  ;Fetch Phase into w4:w4 \n \
        add     w4, [%[phaseInc]++], [%[phase]++]                               ;Add LSBs \n \
        mov     [%[phase]], w4                                                  ;Fetch Phase into w4:w4 \n \
        addc    w4, [%[phaseInc]++], [%[phase]++]                               ;Add MSBs \n \
    ;Add scaled side oscillator 2 to Accumulator A                              ;\n \
        mac     w4 * %[levelSide], A                                            ;AccA += side oscillator value * side oscillator level \n \
                                                                                ;\n \
    ;Increment side oscillator 3 phase                                          ;\n \
        mov     [%[phase]], w4                                                  ;Fetch Phase into w4:w4 \n \
        add     w4, [%[phaseInc]++], [%[phase]++]                               ;Add LSBs \n \
        mov     [%[phase]], w4                                                  ;Fetch Phase into w4:w4 \n \
        addc    w4, [%[phaseInc]++], [%[phase]++]                               ;Add MSBs \n \
    ;Add scaled side oscillator 3 to Accumulator A                              ;\n \
        mac     w4 * %[levelSide], A                                            ;AccA += side oscillator value * side oscillator level \n \
                                                                                ;\n \
    ;Increment side oscillator 4 phase                                          ;\n \
        mov     [%[phase]], w4                                                  ;Fetch Phase into w4:w4 \n \
        add     w4, [%[phaseInc]++], [%[phase]++]                               ;Add LSBs \n \
        mov     [%[phase]], w4                                                  ;Fetch Phase into w4:w4 \n \
        addc    w4, [%[phaseInc]++], [%[phase]++]                               ;Add MSBs \n \
    ;Add scaled side oscillator 4 to Accumulator A                              ;\n \
        mac     w4 * %[levelSide], A                                            ;AccA += side oscillator value * side oscillator level \n \
                                                                                ;\n \
    ;Increment side oscillator 5 phase                                          ;\n \
        mov     [%[phase]], w4                                                  ;Fetch Phase into w4:w4 \n \
        add     w4, [%[phaseInc]++], [%[phase]++]                               ;Add LSBs \n \
        mov     [%[phase]], w4                                                  ;Fetch Phase into w4:w4 \n \
        addc    w4, [%[phaseInc]++], [%[phase]++]                               ;Add MSBs \n \
    ;Add scaled side oscillator 5 to Accumulator A                              ;\n \
        mac     w4 * %[levelSide], A                                            ;AccA += side oscillator value * side oscillator level \n \
                                                                                ;\n \
    ;Increment side oscillator 6 phase                                          ;\n \
        mov     [%[phase]], w4                                                  ;Fetch Phase into w4:w4 \n \
        add     w4, [%[phaseInc]++], [%[phase]++]                               ;Add LSBs \n \
        mov     [%[phase]], w4                                                  ;Fetch Phase into w4:w4 \n \
        addc    w4, [%[phaseInc]++], [%[phase]++]                               ;Add MSBs \n \
    ;Add scaled side oscillator 6 to Accumulator A                              ;\n \
        mac     w4 * %[levelSide], A                                            ;AccA += side oscillator value * side oscillator level \n \
                                                                                ;\n \
        sac.r   A, #1, %[Output]                                                ;Store AccA/2 in Output \n \
                                                                                ;\n \
        ;35 cycles total"
            : [Output]"=r"(output), [phase]"+r"(phase), [phaseInc]"+r"(phaseInc) /*out*/
            : [levelCenter]"z"(levelCenter), [levelSide]"z"(levelSide) /*in*/
            : "w4" /*clobbered*/
            );

    // Apply 4th order Butterworth highpass filter to suppress sub-harmonics caused by aliasing

    // Cache pointers for use with inline assembly
    _Q15 * filterState = state->filter[0].state;
    const _Q15 * filterCoeffs = params->filterCoeffs1;

    // First SOS
    __asm__ volatile(
            "\
    ;v1 = a[0] * s[0] - a[1] * s[1] + a[1] * x                                  ;\n \
        movsac  A, [%[s]]+=2, w4, [%[a]]+=2, w5                                 ;Prefetch s[0] and a[0] \n \
        mpy     w4 * w5, A, [%[s]]-=2, w4, [%[a]]+=2, w5                        ;AccA = s[0] * a[0], prefetch s[1] and a[1] \n \
        msc     w4 * w5, A                                                      ;AccA -= s[1] * a[1], prefetch x and keep a[1] \n \
        mac     %[x] * w5, A, [%[a]]+=2, w5                                     ;AccA += x * a[1], prefetch a[2] \n \
        sac.r   A, #0, w4                                                       ;Store v1 in w4 \n \
                                                                                ;\n \
    ;s[0] = 2 * v1 - s[0] = 2 * ( v1 - 0.5 * s[0] )                             ;\n \
        lac     [%[s]], #1, B                                                   ;AccB = 0.5 * s[0] \n \
        sub     A                                                               ;AccA = v1 - 0.5 * s[0] \n \
        sac.r   A, #-1, [%[s]++]                                                ;Store AccA * 2 in s[0] \n \
                                                                                ;\n \
    ;v2 = s[1] + g * v1                                                         ;\n \
        mpy     w4 * w5, A, [%[a]]-=6, w5                                       ;AccA = v1 * g, prefetch a[3] \n \
        add     [%[s]], #3, A                                                   ;AccA += s[1] (Q0.15 --> Q3.12) \n \
        sac.r   A, #0, w0                                                       ;Store v2 in w5 (Q3.12) \n \
                                                                                ;\n \
    ;s[1] = 2 * v2 - s[1] = 2 * ( v2 - 0.5 * s[1] )                             ;\n \
        lac     [%[s]], #4, B                                                   ;AccB = 0.5 * s[1] (Q0.15 --> Q3.12) \n \
        sub     A                                                               ;AccA = v2 - 0.5 * s[1] \n \
        sac.r   A, #-4, [%[s]--]                                                ;Store AccA * 2 in s[1] (Q3.28 --> Q0.15) \n \
                                                                                ;\n \
    ;y = x - k * v1 - v2                                                        ;\n \
        lac     %[x], #3, A                                                     ;AccA = x (Q0.15 --> Q3.12) \n \
        lac     w0, #0, B                                                       ;AccB = v2 (Q3.12) \n \
        sub     A                                                               ;AccA = x - v2 (Q3.12) \n \
        msc     w4 * w5, A, [%[s]]+=2, w4, [%[a]]+=2, w5                        ;AccA -= v1 * a[3], prefetch s[0] and a[0] for next do-loop iteration \n \
                                                                                ;\n \
        sac.r   A, #-3, %[x]                                                    ;Store AccA in x, increment x pointer (Q3.28 --> Q0.15) \n \
                                                                                ;\n \
        ; 19 cycles total"
            : [x]"+z"(output), [s]"+x"(filterState), [a]"+y"(filterCoeffs) /*out*/
            : /*in*/
            : "w0", "w4", "w5" /*clobbered*/
            );

    // Cache pointers for use with inline assembly
    filterState = state->filter[1].state;
    filterCoeffs = params->filterCoeffs2;

    // Second SOS
    __asm__ volatile(
            "\
    ;v1 = a[0] * s[0] - a[1] * s[1] + a[1] * x                                  ;\n \
        movsac  A, [%[s]]+=2, w4, [%[a]]+=2, w5                                 ;Prefetch s[0] and a[0] \n \
        mpy     w4 * w5, A, [%[s]]-=2, w4, [%[a]]+=2, w5                        ;AccA = s[0] * a[0], prefetch s[1] and a[1] \n \
        msc     w4 * w5, A                                                      ;AccA -= s[1] * a[1], prefetch x and keep a[1] \n \
        mac     %[x] * w5, A, [%[a]]+=2, w5                                     ;AccA += x * a[1], prefetch a[2] \n \
        sac.r   A, #0, w4                                                       ;Store v1 in w4 \n \
                                                                                ;\n \
    ;s[0] = 2 * v1 - s[0] = 2 * ( v1 - 0.5 * s[0] )                             ;\n \
        lac     [%[s]], #1, B                                                   ;AccB = 0.5 * s[0] \n \
        sub     A                                                               ;AccA = v1 - 0.5 * s[0] \n \
        sac.r   A, #-1, [%[s]++]                                                ;Store AccA * 2 in s[0] \n \
                                                                                ;\n \
    ;v2 = s[1] + g * v1                                                         ;\n \
        mpy     w4 * w5, A, [%[a]]-=6, w5                                       ;AccA = v1 * g, prefetch a[3] \n \
        add     [%[s]], #3, A                                                   ;AccA += s[1] (Q0.15 --> Q3.12) \n \
        sac.r   A, #0, w0                                                       ;Store v2 in w5 (Q3.12) \n \
                                                                                ;\n \
    ;s[1] = 2 * v2 - s[1] = 2 * ( v2 - 0.5 * s[1] )                             ;\n \
        lac     [%[s]], #4, B                                                   ;AccB = 0.5 * s[1] (Q0.15 --> Q3.12) \n \
        sub     A                                                               ;AccA = v2 - 0.5 * s[1] \n \
        sac.r   A, #-4, [%[s]--]                                                ;Store AccA * 2 in s[1] (Q3.28 --> Q0.15) \n \
                                                                                ;\n \
    ;y = x - k * v1 - v2                                                        ;\n \
        lac     %[x], #3, A                                                     ;AccA = x (Q0.15 --> Q3.12) \n \
        lac     w0, #0, B                                                       ;AccB = v2 (Q3.12) \n \
        sub     A                                                               ;AccA = x - v2 (Q3.12) \n \
        msc     w4 * w5, A, [%[s]]+=2, w4, [%[a]]+=2, w5                        ;AccA -= v1 * a[3], prefetch s[0] and a[0] for next do-loop iteration \n \
                                                                                ;\n \
        sac.r   A, #-3, %[x]                                                    ;Store AccA in x, increment x pointer (Q3.28 --> Q0.15) \n \
                                                                                ;\n \
        ; 19 cycles total"
            : [x]"+z"(output), [s]"+x"(filterState), [a]"+y"(filterCoeffs) /*out*/
            : /*in*/
            : "w0", "w4", "w5" /*clobbered*/
            );

    return output;
}

#endif	/* VCO_SAW_H */

