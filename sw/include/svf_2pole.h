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
 * @file svf_2pole.h
 * @brief Implementation of 2-pole state-variable filter (SVF)
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#ifndef SVF_2POLE_H
#define	SVF_2POLE_H

#include <stdint.h>
#include "fp_lib_types.h"
#include "fp_lib_interp.h"
#include "fp_lib_typeconv.h"
#include "svf_2pole_types.h"
#include "block_len_def.h"

/**
 * @brief Lookup table for note to alpha conversion
 * Defined in iir_1pole.c
 */
extern const _Q15 noteToSVF2PoleGTable[257];


///**
// * @brief Calculation of SVF parameter "g" for given filter frequency
// * 
// * Calculation of SVF parameter "g" according to the notation as found in:\n
// * https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf
// * @note The result is returned as Q0.15 number, but has to be considered as Q3.12 number
// * @param note Filter frequency on MIDI note scale given in half-cents
// * @return SVF parameter "g" in Q3.12 format
// */
//static inline _Q15 calcG(const uint16_t note)
//{
//    // Calculate G by table interpolation
//    return interpLUT_256_Q15(
//                             noteToSVF2PoleGTable,
//                             note);
//}
//
///**
// * @brief Calculation of SVF parameter "k" for given filter resonance
// * 
// * Calculation of SVF parameter "g" according to the notation as found in:\n
// * https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf
// * @note The result is returned as Q0.15 number, but has to be considered as Q3.12 number
// * @param resonance Filter resonance in Q0.16 format
// * @return SVF parameter "k" in Q3.12 format
// */
//static inline _Q15 calcK(const _Q16 resonance)
//{
//    // k = 2 - 2 * resonance = 2 * (1 - resonance),  0 < resonance < 1 --> 0 < k < 2
//    // Combine the following steps:
//    // 1. Calculate 1 - resonance --> Approximate by 1-x by ~x (x is Q0.16)
//    // 2. Convert (1-resonance) from Q0.16 to Q3.12 --> right-shift by four bits
//    // 3. Multiply (1-resonance) by 2 --> left-shift by one bit)
//    return (~resonance) >> 3; // Q3.12
//}

/**
 * @brief Calculation of SVF parameters
 * 
 * Calculation of SVF parameters "a" according to the notation as found in:\n
 * https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf
 * @param note Filter frequency on MIDI note scale given in half-cents
 * @param resonance Filter resonance in Q0.16 format
 * @param coeffs Finalized SVF parameters in Q0.15/Q3.12 format
 */
static inline void calcCoeffs(
                              const int16_t note,
                              const _Q16 resonance,
                              _Q15 * coeffs)
{
    // Calculate G by table interpolation
    const _Q15 g = interpLUT_256_Q15(
                             noteToSVF2PoleGTable,
                             convert_Q15_Q16_Naive(note));

    // k = 2 - 2 * resonance = 2 * (1 - resonance),  0 < resonance < 1 --> 0 < k < 2
    // Combine the following steps:
    // 1. Calculate 1 - resonance --> Approximate 1-x by ~x (x is Q0.16)
    // 2. Convert (1-resonance) from Q0.16 to Q3.12 --> right-shift by four bits
    // 3. Multiply (1-resonance) by 2 --> left-shift by one bit)
    const _Q15 k = (~resonance) >> 3; // Q3.12

    // a(1) = 1 / ((g + k) * g + 1);
    // a(2) = g * a(1);
    _Q15 temp = k + g;

    __asm__ volatile(
            "\
        mpy     %[temp] * %[g], A       ;AccA = (g + k) * g (Q6.25) \n \
        sac.r   A, #-3, %[temp]         ;Store AccA (Q6.25 --> Q3.12) \n \
        "
            : [temp]"+z"(temp) /*out*/
            : [g]"z"(g) /*in*/
            : /*clobbered*/
            );

    temp += 4096; // (g + k) * g + 1 (Q3.12)

    // a(1) = 1 / ((g + k) * g + 1);
    coeffs[0] = __builtin_divf(4095, temp);

    // a(2) = g  / ((g + k) * g + 1);
    coeffs[1] = __builtin_divf(g, temp);

    coeffs[2] = g;
    coeffs[3] = k;
}

/**
 * @brief In-place filtering of one block of samples with SVF lowpass output
 * 
 * Calculation of SVF output according to the notation as found in:\n
 * https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf
 * @param coeffs Struct holding SVF coefficients
 * @param state Struct holding SVF state
 * @param data  Buffer of BLOCK_LEN samples to be filtered
 */
static inline void calcLP2PoleBlockInplace(
                                           const _Q15 * coeffs,
                                           SVF2PoleState * const state,
                                           _Q15 * data)
{
    // Cache pointer for use with inline assembly
    _Q15 * filterState = state->state;

    // Loop all samples
    __asm__ volatile(
            "\
        movsac  A, [%[s]]+=2, w4, [%[a]]+=2, w5                                 ;Prefetch s[0] and a[0] \n \
                                                                                ;\n \
    ;Start processing loop                                                      ;\n \
        do #%[Len]-1, CalcLP2Pole_%=                                            ;\n \
                                                                                ;\n \
    ;v1 = a[0] * s[0] - a[1] * s[1] + a[1] * x                                  ;\n \
        mpy     w4 * w5, A, [%[s]]-=2, w4, [%[a]]+=2, w5                        ;AccA = s[0] * a[0], prefetch s[1] and a[1] \n \
        msc     w4 * w5, A, [%[x]], w4                                          ;AccA -= s[1] * a[1], prefetch x and keep a[1] \n \
        mac     w4 * w5, A, [%[a]]-=4, w5                                       ;AccA += x * a[1], prefetch a[2] \n \
        sac.r   A, #0, w4                                                       ;Store AccA in w4 \n \
                                                                                ;\n \
    ;s[0] = 2 * v1 - s[0] = 2 * ( v1 - 0.5 * s[0] )                             ;\n \
        lac     [%[s]], #1, B                                                   ;AccB = 0.5 * s[0] \n \
        sub     A                                                               ;AccA = v1 - 0.5 * s[0] \n \
        sac.r   A, #-1, [%[s]]                                                  ;Store AccA * 2 in s[0] \n \
                                                                                ;\n \
    ;v2 = s[1] + g * v1                                                         ;\n \
        mpy     w4 * w5, A, [%[s]]+=2, w4, [%[a]]+=2, w5                        ;AccA = v1 * a[2], prefetch s[0] and a[0] for next do-loop iteration \n \
        add     [%[s]], #3, A                                                   ;AccA += s[1] (Q0.15 --> Q3.12) \n \
        sac.r   A, #-3, [%[x]++]                                                ;Store AccA in x (Q3.28 --> Q0.15), increment x pointer \n \
                                                                                ;\n \
    ;s[1] = 2 * v2 - s[1] = 2 * ( v2 - 0.5 * s[1] )                             ;\n \
        lac     [%[s]], #4, B                                                   ;AccB = 0.5 * s[1] (Q0.15 --> Q3.12) \n \
        sub     A                                                               ;AccA = v2 - 0.5 * s[1] \n \
                                                                                ;\n \
    CalcLP2Pole_%=:                                                             ;\n \
        sac.r   A, #-4, [%[s]]                                                  ;Store AccA * 2 in s[1] (Q3.28 --> Q0.15) \n \
                                                                                ;\n \
        ; 3 + 13N cycles total"
            : [x]"+x"(data), [s]"+x"(filterState), [a]"+y"(coeffs) /*out*/
            : [Len]"i"(BLOCK_LEN) /*in*/
            : "w4", "w5" /*clobbered*/
            );
}

/**
 * @brief In-place filtering of one block of samples with SVF bandpass output
 * 
 * Calculation of SVF output according to the notation as found in:\n
 * https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf
 * @param coeffs Struct holding SVF coefficients
 * @param state Struct holding SVF state
 * @param data  Buffer of BLOCK_LEN samples to be filtered
 */
static inline void calcBP2PoleBlockInplace(
                                           const _Q15 * coeffs,
                                           SVF2PoleState * const state,
                                           _Q15 * data)
{
    // Cache pointer for use with inline assembly
    _Q15 * filterState = state->state;

    // Loop all samples
    __asm__ volatile(
            "\
        movsac  A, [%[s]]+=2, w4, [%[a]]+=2, w5                                 ;Prefetch s[0] and a[0] \n \
                                                                                ;\n \
    ;Start processing loop                                                      ;\n \
        do #%[Len]-1, CalcBP2Pole_%=                                            ;\n \
                                                                                ;\n \
    ;v1 = a[0] * s[0] - a[1] * s[1] + a[1] * x                                  ;\n \
        mpy     w4 * w5, A, [%[s]]-=2, w4, [%[a]]+=2, w5                        ;AccA = s[0] * a[0], prefetch s[1] and a[1] \n \
        msc     w4 * w5, A, [%[x]], w4                                          ;AccA -= s[1] * a[1], prefetch x and keep a[1] \n \
        mac     w4 * w5, A, [%[a]]-=4, w5                                       ;AccA += x * a[1], prefetch a[2] \n \
        sac.r   A, #0, w4                                                       ;Store AccA in w4 \n \
        sac.r   A, #0, [%[x]++]                                                 ;Store AccA in x, increment x pointer \n \
                                                                                ;\n \
    ;s[0] = 2 * v1 - s[0] = 2 * ( v1 - 0.5 * s[0] )                             ;\n \
        lac     [%[s]], #1, B                                                   ;AccB = 0.5 * s[0] \n \
        sub     A                                                               ;AccA = v1 - 0.5 * s[0] \n \
        sac.r   A, #-1, [%[s]]                                                  ;Store AccA * 2 in s[0] \n \
                                                                                ;\n \
    ;v2 = s[1] + g * v1                                                         ;\n \
        mpy     w4 * w5, A, [%[s]]+=2, w4, [%[a]]+=2, w5                        ;AccA = v1 * g, prefetch s[0] and a[0] for next do-loop iteration \n \
        add     [%[s]], #3, A                                                   ;AccA += s[1] (Q0.15 --> Q3.12) \n \
                                                                                ;\n \
    ;s[1] = 2 * v2 - s[1] = 2 * ( v2 - 0.5 * s[1] )                             ;\n \
        lac     [%[s]], #4, B                                                   ;AccB = 0.5 * s[1] (Q0.15 --> Q3.12) \n \
        sub     A                                                               ;AccA = v2 - 0.5 * s[1] \n \
                                                                                ;\n \
    CalcBP2Pole_%=:                                                             ;\n \
        sac.r   A, #-4, [%[s]]                                                  ;Store AccA * 2 in s[1] (Q3.28 --> Q0.15) \n \
                                                                                ;\n \
        ; 3 + 13N cycles total"
            : [x]"+x"(data), [s]"+x"(filterState), [a]"+y"(coeffs) /*out*/
            : [Len]"i"(BLOCK_LEN) /*in*/
            : "w4", "w5" /*clobbered*/
            );
}

/**
 * @brief In-place filtering of one block of samples with SVF highpass output
 * 
 * Calculation of SVF output according to the notation as found in:\n
 * https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf
 * @param coeffs Struct holding SVF coefficients
 * @param state Struct holding SVF state
 * @param data  Buffer of BLOCK_LEN samples to be filtered
 */
static inline void calcHP2PoleBlockInplace(
                                           const _Q15 * coeffs,
                                           SVF2PoleState * const state,
                                           _Q15 * data)
{
    // Cache pointer for use with inline assembly
    _Q15 * filterState = state->state;

    // Loop all samples
    __asm__ volatile(
            "\
        movsac  A, [%[s]]+=2, w4, [%[a]]+=2, w5                                 ;Prefetch s[0] and a[0] \n \
                                                                                ;\n \
    ;Start processing loop                                                      ;\n \
        do #%[Len]-1, CalcHP2Pole_%=                                               ;\n \
                                                                                ;\n \
    ;v1 = a[0] * s[0] - a[1] * s[1] + a[1] * x                                  ;\n \
        mpy     w4 * w5, A, [%[s]]-=2, w4, [%[a]]+=2, w5                        ;AccA = s[0] * a[0], prefetch s[1] and a[1] \n \
        msc     w4 * w5, A, [%[x]], w4                                          ;AccA -= s[1] * a[1], prefetch x and keep a[1] \n \
        mac     w4 * w5, A, [%[a]]+=2, w5                                       ;AccA += x * a[1], prefetch a[2] \n \
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
        lac     [%[x]], #3, A                                                   ;AccA = x (Q0.15 --> Q3.12) \n \
        lac     w0, #0, B                                                       ;AccB = v2 (Q3.12) \n \
        sub     A                                                               ;AccA = x - v2 (Q3.12) \n \
        msc     w4 * w5, A, [%[s]]+=2, w4, [%[a]]+=2, w5                        ;AccA -= v1 * a[3], prefetch s[0] and a[0] for next do-loop iteration \n \
                                                                                ;\n \
    CalcHP2Pole_%=:                                                             ;\n \
        sac.r   A, #-3, [%[x]++]                                                ;Store AccA in x, increment x pointer (Q3.28 --> Q0.15) \n \
                                                                                ;\n \
        ; 4 + 18N cycles total"
            : [x]"+x"(data), [s]"+x"(filterState), [a]"+y"(coeffs) /*out*/
            : [Len]"i"(BLOCK_LEN) /*in*/
            : "w0", "w4", "w5" /*clobbered*/
            );
}

#endif

