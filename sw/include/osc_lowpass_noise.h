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
 * @file osc_lowpass_noise.h
 * @brief Implementation of lowpass noise oscillator
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#ifndef OSC_LOWPASS_NOISE_H
#define	OSC_LOWPASS_NOISE_H

#include "rand.h"
#include "svf_2pole.h"
#include "osc_lowpass_noise_types.h"
#include "fp_lib_types.h"
#include <stdint.h>

/**
 * @brief Calculate parameters for lowpass noise oscillator
 * @param pitch Pitch on MIDI scale given in half-cents
 * @param shape1 Shape parameter in Q0.16 format representing the lowpass filter frequency
 * @param shape2 Shape parameter in Q0.16 format representing the lowpass filter resonance
 * @param params Struct holding lowpass noise oscillator parameters
 */
inline static void calcOscLowPassNoiseParams(
                                             const int16_t note,
                                             const _Q16 shape1,
                                             const _Q16 shape2,
                                             OscLowPassNoiseParams * const params)
{
    // Combine note and oscillator shape 1 to one frequency value using saturated addition
    int16_t cutOff;
    __asm__ volatile(
            "\
        lac     %[note], #0, A    ;AccA = Pitch (converted to quarter-cents) \n \
        add     %[freq], #0, A     ;AccA = Pitch + Freq \n \
        sac.r   A, #0, %[cutOff]   ;Store AccA  \n \
        "
            : [cutOff]"=r"(cutOff) /*out*/
            : [note]"r"(note), [freq]"r"(convert_Q16_Q15(shape1)) /*in*/
            : /*clobbered*/
            );

    // Calculate filter coefficients for 2-pole low pass
    calcCoeffs(
               cutOff,
               shape2,
               params->filterCoeffs);
}

/**
 * @brief Calculate one sample of lowpass noise oscillator waveform
 * 
 * Update the oscillator state and calculate one waveform sample from the updated state
 * @param params Struct holding lowpass noise oscillator parameters
 * @param state Struct holding lowpass noise oscillator state
 * @return Lowpass noise oscillator waveform sample in Q0.15 format
 */
inline static _Q15 calcOscLowPassNoiseSample(
                                             const OscLowPassNoiseParams * const params,
                                             OscLowPassNoiseState * const state)
{
    // Get white noise sample
    _Q15 output = rand();


    // Cache pointers for use in inline assembly
    const _Q15 * filterCoeffs = params->filterCoeffs;
    _Q15 * filterState = state->filter.state;

    // Filter white noise
    __asm__ volatile(
            "\
     ;v1 = a[0] * s[0] - a[1] * s[1] + a[1] * x             ;\n \
        movsac  A, [%[s]]+=2, w4, [%[a]]+=2, w5             ;Prefetch s[0] and a[0] \n \
        mpy     w4 * w5, A, [%[s]]-=2, w4, [%[a]]+=2, w5    ;AccA = s[0] * a[0], prefetch s[1] and a[1] \n \
        msc     w4 * w5, A                                  ;AccA -= s[1] * a[1], prefetch x and keep a[1] \n \
        mac     %[x] * w5, A, [%[a]]-=4, w5                 ;AccA += x * a[1], prefetch a[2] \n \
        sac.r   A, #0, w4                                   ;Store AccA in w4 \n \
                                                            ;\n \
    ;s[0] = 2 * v1 - s[0] = 2 * ( v1 - 0.5 * s[0] )         ;\n \
        lac     [%[s]], #1, B                               ;AccB = 0.5 * s[0] \n \
        sub     A                                           ;AccA = v1 - 0.5 * s[0] \n \
        sac.r   A, #-1, [%[s]]                              ;Store AccA * 2 in s[0] \n \
                                                            ;\n \
    ;v2 = s[1] + g * v1                                     ;\n \
        mpy     w4 * w5, A, [%[s]]+=2, w4, [%[a]]+=2, w5    ;AccA = v1 * a[2], prefetch s[0] and a[0] for next do-loop iteration \n \
        add     [%[s]], #3, A                               ;AccA += s[1] (Q0.15 --> Q3.12) \n \
        sac.r   A, #-3, %[x]                                ;Store AccA in x (Q3.28 --> Q0.15), increment x pointer \n \
                                                            ;\n \
    ;s[1] = 2 * v2 - s[1] = 2 * ( v2 - 0.5 * s[1] )         ;\n \
        lac     [%[s]], #4, B                               ;AccB = 0.5 * s[1] (Q0.15 --> Q3.12) \n \
        sub     A                                           ;AccA = v2 - 0.5 * s[1] \n \
        sac.r   A, #-4, [%[s]]                              ;Store AccA * 2 in s[1] (Q3.28 --> Q0.15) \n \
                                                            ;\n \
        ; 14 cycles total"
            : [x]"+z"(output), [s]"+x"(filterState), [a]"+y"(filterCoeffs) /*out*/
            : /*in*/
            : "w4", "w5" /*clobbered*/
            );

    return output;
}


#endif
