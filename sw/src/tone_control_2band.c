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
 * @file tone_control_2bands.c
 * @brief Implementation of two band tone control
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#include "tone_control_2band.h"
#include "tone_control_2band_types.h"
#include "fp_lib_types.h"
#include "fp_lib_mul.h"
#include "block_len_def.h"

/**
 * @brief Calculation of low shelf filter parameters
 * 
 * Calculation of low shelf filter parameters as described in:\n
 * https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf
 * @note Filter corner frequency is 200 Hz @ 44100 Hz sample rate
 * @param bass Bass parameter Q0.15 format
 * @param coeffs Finalized shelf filter parameters in Q0.15 format
 */
static inline void calcBassCoeffs(
                              const _Q15 bass,
                              _Q15 * const coeffs)
{
    coeffs[0] = 30977;
    coeffs[1] = 883;
    coeffs[2] = 25;
 
    // Coeffs 3,4,5 are divided by two
    const _Q15 gain = mul_Q15_Q15(bass, 13573);
    coeffs[3] = 16384;
    coeffs[4] = gain;
    coeffs[5] = (mul_Q15_Q15(gain, gain) >> 1) + coeffs[4];
}

/**
 * @brief Calculation of high shelf filter parameters
 * 
 * Calculation of high shelf filter parameters as described in:\n
 * https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf
 * @note Filter corner frequency is 1000 Hz @ 44100 Hz sample rate
 * @param treble Treble parameter Q0.15 format
 * @param coeffs Finalized shelf filter parameters in Q0.15 format
 */
static inline void calcTrebleCoeffs(
                              const _Q15 treble,
                              _Q15 * const coeffs)
{
    coeffs[0] = 25062;
    coeffs[1] = 3595;
    coeffs[2] = 516;
 
    // Coeffs 3,4,5 are divided by two
    const _Q15 gain = mul_Q15_Q15(treble, 9598) + 23170;
    coeffs[3] = mul_Q15_Q15(gain, gain);
    coeffs[4] = (mul_Q15_Q15(23170, gain) - coeffs[3]) << 1;
    coeffs[5] = 16384 - coeffs[3];
}

/**
 * @brief In-place filtering of one block of samples with SVF shelf filter output
 * 
 * Calculation of SVF output according to the notation as found in:\n
 * https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf
 * @param coeffs SVF coefficients in Q0.15 format
 * @param state SVF state in Q0.15 format
 * @param data  Buffer of BLOCK_LEN samples to be filtered
 */
static inline void calcShelf2PoleBlockInplace(
                                           const _Q15 * coeffs,
                                           _Q15 * state,
                                           _Q15 * xBuffer,
                                           _Q15 * data)
{    
    xBuffer[0] = state[0];
    xBuffer[1] = state[1];
    
    _Q15 * v = &xBuffer[2];

    // Loop all samples
    // @todo Utilize x prefetch for v
    __asm__ volatile(
            "\
        movsac  A, [%[s]]+=2, w4, [%[a]]+=2, w5                                 ;Prefetch s[0+1] and a[0+1] \n \
                                                                                ;\n \
    ;Start processing loop                                                      ;\n \
        do #%[Len]-1, CalcLS2Pole_%=                                            ;\n \
                                                                                ;\n \
    ;v1 = a[0] * s[0] - a[1] * s[1] + a[1] * x                                  ;\n \
        mpy     w4 * w5, A, [%[s]]-=2, w4, [%[a]]+=2, w5                        ;AccA = s[0] * a[0], prefetch s[1-1] and a[1-1] \n \
        msc     w4 * w5, A                                                      ;AccA -= s[1] * a[1], prefetch x and keep a[1] \n \
        mov     [%[x]], w4                                                      ;prefetch x \n \
        mac     w4 * w5, A, [%[a]]-=2, w5                                       ;AccA += x * a[1], keep x and prefetch a[2-1] \n \
        sac.r   A, #0, [%[v]++]                                                 ;Store AccA in v[0+1] \n \
                                                                                ;\n \
    ;s[0] = 2 * v1 - s[0] = 2 * ( v1 - 0.5 * s[0] )                             ;\n \
        lac     [%[s]++], #1, B                                                 ;AccB = 0.5 * s[0+1] \n \
        sub     A                                                               ;AccA = v1 - 0.5 * s[0] \n \
                                                                                ;\n \
    ;v2 = x * a[2] - a[2] * s[1] + s[1] + s[0] * a[1]                           ;\n \
        mpy     w4 * w5, B, [%[s]]-=2, w4                                       ;AccB = x * a[2], prefetch s[1-1] and keep a[2-1] \n \
        add     w4, #0, B                                                       ;AccB += s[1] \n \
        msc     w4 * w5, B, [%[s]], w4, [%[a]]+=6, w5                           ;AccB -= s[1] * a[2], prefetch s[0] and a[1+3] \n \
        mac     w4 * w5, B, [%[a]]+=2, w5                                       ;AccB += s[0] * a[1], keep x and prefetch a[4+1] \n \
        sac.r   B, #0, [%[v]--]                                                 ;Store AccB in v[1-1] \n \
                                                                                ;\n \
        sac.r   A, #-1, [%[s]++]                                                ;Store AccA * 2 in s[0+1] \n \
    ;s[1] = 2 * v2 - s[1] = 2 * ( v2 - 0.5 * s[1] )                             ;\n \
        lac     [%[s]], #1, A                                                   ;AccA = 0.5 * s[1] (Q0.15 --> Q3.12) \n \
        sub     B                                                               ;AccA = v2 - 0.5 * s[1] \n \
        sac.r   B, #-1, [%[s]--]                                                ;Store AccB * 2 in s[1-1] \n \
                                                                                ;\n \
    ;y = x + a[3] * v1 + a[4] * v2                                              ;\n \
        mov     [%[v]++], w4                                                    ;prefetch v1 \n \
        mpy     w4 * w5, A, [%[a]]-=4, w5                                       ;AccA += v1 * a[4], keep v2 and prefetch a[5-2] \n \
        mov     [%[v]--], w4                                                    ;prefetch v2 \n \
        mac     w4 * w5, A, [%[a]]-=6, w5                                       ;AccA += v2 * a[5], keep v2 and prefetch a[3-3] \n \
        mov     [%[x]], w4                                                      ;prefetch x \n \
        mac     w4 * w5, A, [%[s]]+=2, w4, [%[a]]+=2, w5                        ;AccA += x * a[3], prefetch s[0+1] and a[0+1] \n \
                                                                                ;\n \
    CalcLS2Pole_%=:                                                             ;\n \
        sac.r   A, #-1, [%[x]++]                                                ;Store AccA * 2 in x (* 2 because of filter coefficient scaling) \n \
                                                                                ;\n \
        ; 3 + 13N cycles total"
            : [x]"+r"(data), [s]"+x"(xBuffer), [a]"+y"(coeffs), [v]"+x"(v) /*out*/
            : [Len]"i"(BLOCK_LEN) /*in*/
            : "w4", "w5" /*clobbered*/
            );

    state[0] = xBuffer[-1];
    state[1] = xBuffer[0];
}

/**
 * In-place calculation of 2-band stereo tone control
 * 
 * @param params Struct holding tone control parameters
 * @param state Struct holding tone control state
 * @param xBuffer Work buffer of size >= 4 allocated in x memory
 * @param yBuffer Work buffer of size >= 6 allocated in y memory
 * @param bufferLeft Input/Output buffer for left stereo channel in Q0.15 format
 * @param bufferRight Input/Output buffer for right stereo channel in Q0.15 format
 */
void calcToneControl2Band(
        const ToneControl2BandParams * const params,
        ToneControl2BandState * const state,
        _Q15 * const xBuffer,
        _Q15 * const yBuffer,
        _Q15 * const bufferLeft,
        _Q15 * const bufferRight)
{
    // Treble / high shelf filter
    calcTrebleCoeffs(
            params->treble,
            yBuffer);

    calcShelf2PoleBlockInplace(
            yBuffer,
            state->trebleStateLeft,
            xBuffer,
            bufferLeft);

    calcShelf2PoleBlockInplace(
            yBuffer,
            state->trebleStateRight,
            xBuffer,
            bufferRight);

    // Bass / low shelf filter
    calcBassCoeffs(
            params->bass,
            yBuffer);

    calcShelf2PoleBlockInplace(
            yBuffer,
            state->bassStateLeft,
            xBuffer,
            bufferLeft);

    calcShelf2PoleBlockInplace(
            yBuffer,
            state->bassStateRight,
            xBuffer,
            bufferRight);
}



