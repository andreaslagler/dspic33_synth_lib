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
 * @file iir_1pole.h
 * @brief Implementation of 1-pole IIR filter
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#ifndef IIR_1POLE_H
#define	IIR_1POLE_H

#include "IIR_1Pole_types.h"
#include "fp_lib_types.h"
#include "fp_lib_interp.h"
#include <stdint.h>
#include "block_len_def.h"

/**
 * @brief Lookup table for note to alpha conversion
 * Defined in iir_1pole.c
 */
extern const _Q15 noteToIIR1PoleAlphaTable[257];

/**
 * @brief Calculate 1-pole IIR filter parameters
 * @param note Note on MIDI scale in half-cents
 * @return Alpha parameter in Q0.15 format
 */
inline static _Q15 calcIIR1PoleAlpha(const uint16_t note)
{
    // Calculate filter coefficient
    return interpLUT_256_Q15(noteToIIR1PoleAlphaTable, note);
}

/**
 * @brief Filter one sample by 1-pole IIR lowpass filter
 * @param alpha Alpha parameter in Q0.15 format
 * @param state Struct holding 1-pole IIR filter struct
 * @param data Data input sample in Q0.15 format
 * @return Filtered output sample in Q0.15 format
 */
inline static _Q15 calcLP1PoleSample(
                                     const _Q15 alpha,
                                     IIROnePoleState * const state,
                                     _Q15 data)
{
    // Calculate 1-pole low pass filter in-place (0 <= a < 1)
    // s = a * s + (1-a) * x
    // y = s;
 
    // Load filter state
    _Q15 stateValue = state->stateValue;
    int16_t stateScaling = state->stateScaling;

    // For accumulator normalizing see section 4.19 of the 16-Bit MCU and DSC Programmer's Reference Manual
    __asm__ volatile(
            "\
        mpy     %[stateValue] * %[alpha], A                                     ;AccA = state * alpha \n \
        neg     %[stateScaling], %[stateScaling]                                ;stateScaling = -stateScaling \n \
        sftac   A, %[stateScaling]                                              ;Un-scale AccA \n \
        mov     %[x], w4                                                        ;Prefetch x into w4  \n \
        msc     w4 * %[alpha], A                                                ;AccA -= x * alpha \n \
        add     w4, #0, A                                                       ;AccA += x \n \
        sac.r   A, #0, %[x]                                                     ;x = AccA \n \
        mov     #ACCAH, w4                                                      ;Dump upper word of AccA into w4 \n \
        fbcl    [w4], %[stateScaling]                                           ;Calculate stateScaling for full scale \n \
        sftac   A, %[stateScaling]                                              ;Normalize AccA \n \
        sac.r   A, #0, %[stateValue]                                            ;stateValue = AccA \n \
                                                                                ;\n \
        ; 11 cycles total"
            : [x]"+r"(data), [stateValue]"+z"(stateValue), [stateScaling]"+r"(stateScaling)/*out*/
            : [alpha]"z"(alpha) /*in*/
            : "w4" /*clobbered*/
            );

    // Store filter state
    state->stateValue = stateValue;
    state->stateScaling = stateScaling;

    return data;
}

/**
 * @brief Filter one sample by 1-pole IIR lowpass filter
 * @param alpha Alpha parameter in Q0.15 format
 * @param state Struct holding 1-pole IIR filter struct
 * @param data Block of data samples to be filtered in Q0.15 format
 */
inline static void calcLP1PoleBlock(
                                    const _Q15 alpha,
                                    IIROnePoleState * const state,
                                    _Q15 * data)
{
    // Calculate 1-pole low pass filter in-place (0 <= a < 1)
    // s = a * s + (1-a) * x(k);
    // y(k) = s;
 
    // Load filter state
    _Q15 stateValue = state->stateValue;
    int16_t stateScaling = state->stateScaling;

    // For accumulator normalizing see section 4.19 of the 16-Bit MCU and DSC Programmer's Reference Manual
    __asm__ volatile(
            "\
        do #%[Len], CalcLP1Pole_%=                                              ;\n \
                                                                                ;\n \
        mpy     %[stateValue] * %[alpha], A                                     ;AccA = stateValue * alpha \n \
        neg     %[stateScaling], %[stateScaling]                                ;stateScaling = -stateScaling \n \
        sftac   A, %[stateScaling]                                              ;De-normalize AccA \n \
        mov     [%[x]], w4                                                      ;Prefetch x(k) into w4 \n \
        msc     w4 * %[alpha], A                                                ;AccA -= x(k) * alpha \n \
        add     w4, #0, A                                                       ;AccA += x(k) \n \
        sac.r   A, #0, [%[x]++]                                                 ;x[k++] = AccA \n \
        mov     #ACCAH, w4                                                      ;Dump upper word of AccA into w4 \n \
        fbcl    [w4], %[stateScaling]                                           ;Calculate stateScaling for full scale \n \
        sftac   A, %[stateScaling]                                              ;Normalize AccA \n \
                                                                                ;\n \
    CalcLP1Pole_%=:                                                             ;\n \
        sac.r   A, #0, %[stateValue]                                            ;stateValue = AccA \n \
                                                                                ;\n \
        ; 2 + 11N cycles total"
            : [x]"+r"(data), [stateValue]"+z"(stateValue), [stateScaling]"+r"(stateScaling)/*out*/
            : [Len]"i"(BLOCK_LEN - 1), [alpha]"z"(alpha) /*in*/
            : "w4" /*clobbered*/
            );

    // Store filter state
    state->stateValue = stateValue;
    state->stateScaling = stateScaling;
}

/**
 * @brief Filter one sample by 1-pole IIR highpass filter
 * @param alpha Alpha parameter in Q0.15 format
 * @param state Struct holding 1-pole IIR filter struct
 * @param data Input data sample in Q0.15 format
 * @return Filtered output sample in Q0.15 format
 */
inline static _Q15 calcHP1PoleSample(
                                     const _Q15 alpha,
                                     IIROnePoleState * const state,
                                     _Q15 data)
{
    // Calculate 1-pole high pass filter in-place (0 <= a < 1)
    // y = a * (x - s))
    // s = x - y
 
    // Load filter state
    _Q15 stateValue = state->stateValue;
    int16_t stateScaling = state->stateScaling;

    // For accumulator normalizing see section 4.19 of the 16-Bit MCU and DSC Programmer's Reference Manual
    __asm__ volatile(
            "\
        mpy.n   %[stateValue] * %[alpha], A                                     ;AccA = -stateValue * alpha \n \
        neg     %[stateScaling], %[stateScaling]                                ;stateScaling = -stateScaling \n \
        sftac   A, %[stateScaling]                                              ;De-normalize AccA \n \
        mov     %[x], w4                                                        ;Prefetch x into w4 \n \
        mac     w4 * %[alpha], A                                                ;AccA += x * alpha \n \
        sac.r   A, #0, %[x]                                                     ;x = AccA \n \
        neg     A                                                               ;AccA = -AccA \n \
        add     w4, #0, A                                                       ;AccA += x \n \
        mov     #ACCAH, w4                                                      ;Dump upper word of AccA into w4 \n \
        fbcl    [w4], %[stateScaling]                                           ;Calculate stateScaling for full scale \n \
        sftac   A, %[stateScaling]                                              ;Normalize AccA \n \
        sac.r   A, #0, %[stateValue]                                            ;State = AccA \n \
                                                                                ;stateValue = AccA \n \
        ; 12 cycles total"
            : [x]"+r"(data), [stateValue]"+z"(stateValue), [stateScaling]"+r"(stateScaling)/*out*/
            : [alpha]"z"(alpha) /*in*/
            : "w4" /*clobbered*/
            );

    // Store filter state
    state->stateValue = stateValue;
    state->stateScaling = stateScaling;

    return data;
}

/**
 * @brief Filter one sample by 1-pole IIR highpass filter
 * @param alpha Alpha parameter in Q0.15 format
 * @param state Struct holding 1-pole IIR filter struct
 * @param data Block of data samples to be filtered in Q0.15 format
 */
inline static void calcHP1PoleBlock(
                                    const _Q15 alpha,
                                    IIROnePoleState * const state,
                                    _Q15 * data)
{
    // Calculate 1-pole high pass filter in-place (0 <= a < 1)
    // y(k) = a * (x(k) - s))
    // s = x(k) - y(k))
 
    // Load filter state
    _Q15 stateValue = state->stateValue;
    int16_t stateScaling = state->stateScaling;

    // For accumulator normalizing see section 4.19 of the 16-Bit MCU and DSC Programmer's Reference Manual
    __asm__ volatile(
            "\
        do #%[Len], CalcHP1Pole_%=                                              ;\n \
                                                                                ;\n \
        mpy.n     %[stateValue] * %[alpha], A                                   ;AccA = state * alpha \n \
        neg     %[stateScaling], %[stateScaling]                                ;stateScaling = -stateScaling \n \
        sftac   A, %[stateScaling]                                              ;De-normalize AccA \n \
        mov     [%[x]], w4                                                      ;Prefetch x(k) into w4 \n \
        mac     w4 * %[alpha], A                                                ;AccA += x(k) * alpha \n \
        sac.r   A, #0, [%[x]++]                                                 ;x[k++] = AccA \n \
        neg     A                                                               ;AccA = -AccA \n \
        add     w4, #0, A                                                       ;AccA += x[k] \n \
        mov     #ACCAH, w4                                                      ;Dump upper word of AccA into w4 \n \
        fbcl    [w4], %[stateScaling]                                           ;Calculate stateScaling for full scale \n \
        sftac   A, %[stateScaling]                                              ;Normalize AccA \n \
                                                                                ;\n \
    CalcHP1Pole_%=:                                                             ;\n \
        sac.r   A, #0, %[stateValue]                                            ;stateValue = AccA \n \
                                                                                ;\n \
        ; 2 + 12 * BLOCK_LEN cycles total"
            : [x]"+r"(data), [stateValue]"+z"(stateValue), [stateScaling]"+z"(stateScaling)/*out*/
            : [Len]"i"(BLOCK_LEN - 1), [alpha]"z"(alpha) /*in*/
            : "w4" /*clobbered*/
            );

    // Store filter state
    state->stateValue = stateValue;
    state->stateScaling = stateScaling;
}

#endif