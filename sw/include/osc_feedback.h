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
 * @file osc_feedback.h
 * @brief Implementation of feedback oscillator
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef OSC_FEEDBACK_H
#define	OSC_FEEDBACK_H

#include "osc_feedback_types.h"
#include "fp_lib_types.h"
#include <stdint.h>

/**
 * @brief Calculate parameters for feedback oscillator
 * @param shape1 The shape1 parameter in Q0.16 format translates into comb filter delay
 * @param shape2 The shape2 parameter in Q0.16 format translates into comb filter feedback
 * @param params Struct holding feedback oscillator parameters
 */
inline static void calcOscFeedbackParams(
                                          const _Q16 shape1,
                                          const _Q16 shape2,
                                          OscFeedbackParams * const params)
{
    // Comb filter delay
    // For calculation of table values see calc_comb_filter_delay_table.m
    static const int16_t combFilterDelay[257] = {
        511, 504, 496, 489, 482, 475, 468, 461, 454, 448, 441, 435, 429, 422, 416, 410,
        404, 398, 393, 387, 381, 376, 370, 365, 359, 354, 349, 344, 339, 334, 329, 324,
        320, 315, 310, 306, 302, 297, 293, 289, 284, 280, 276, 272, 268, 264, 260, 257,
        253, 249, 246, 242, 238, 235, 232, 228, 225, 222, 218, 215, 212, 209, 206, 203,
        200, 197, 194, 191, 189, 186, 183, 181, 178, 175, 173, 170, 168, 165, 163, 161,
        158, 156, 154, 151, 149, 147, 145, 143, 141, 139, 137, 135, 133, 131, 129, 127,
        125, 123, 122, 120, 118, 116, 115, 113, 111, 110, 108, 107, 105, 103, 102, 100,
        99, 98, 96, 95, 93, 92, 91, 89, 88, 87, 86, 84, 83, 82, 81, 79,
        78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63,
        62, 61, 60, 59, 58, 58, 57, 56, 55, 54, 53, 53, 52, 51, 50, 50,
        49, 48, 48, 47, 46, 46, 45, 44, 44, 43, 42, 42, 41, 40, 40, 39,
        39, 38, 38, 37, 37, 36, 35, 35, 34, 34, 33, 33, 33, 32, 32, 31,
        31, 30, 30, 29, 29, 28, 28, 28, 27, 27, 26, 26, 26, 25, 25, 25,
        24, 24, 24, 23, 23, 23, 22, 22, 22, 21, 21, 21, 20, 20, 20, 19,
        19, 19, 19, 18, 18, 18, 18, 17, 17, 17, 17, 16, 16, 16, 16, 15,
        15, 15, 15, 15, 14, 14, 14, 14, 13, 13, 13, 13, 13, 13, 12, 12,
        12
    };

    params->delay = interpLUT_256_Q15(
                                      combFilterDelay,
                                      shape1);

    // Combfilter feedback
    params->feedback = convert_Q16_Q15(shape2);
}

/**
 * @brief Calculate one sample of feedback waveform for given oscillator phase
 * 
 * This waveform is a naive saw wave filtered by a recursive comb filter with variable delay and feedback
 * @param phase Oscillator instantaneous phase in Q0.16 format
 * @param params Struct holding feedback oscillator parameters
 * @param state Struct holding feedback oscillator state
 * @return Feedback waveform sample in Q0.15 format
 */
inline static _Q15 calcOscFeedback(
                                   const _Q16 phase,
                                   const OscFeedbackParams * const params,
                                   OscFeedbackState * const state)
{
    // Feedback oscillator feeds a saw wave into a recursive comb filter with variable delay and feedback
    // NB:
    // - Feedback is negative, so first peak in frequency response is at 0.5 * sample rate / delay
    // - Input signal is normalized to (1 - feedback), so comb filter clipping is avoided

    // TODO think about fractional delay

    // Cache delay line read position
    uint16_t readPos = state->readPos;

    // Read from delay line
    _Q15 output = state->delayLine[readPos];

    const _Q15 feedback = params->feedback;

    // Calculate next input into delay line
    __asm__ volatile(
            "\
        lac     %[Saw], #0, A                                                   ;AccA = Saw \n \
        ;msc     %[Saw] * %[Feedback], A                                         ;AccA += Saw * feedback \n \
        mac     %[InOut] * %[Feedback], A                                       ;AccA -= delay line output * feedback \n \
        sac.r   A, #0, %[InOut]                                                 ;Out = AccA \n \
                                                                                ;\n \
        ; 4 cycles total"
            : [InOut]"+z"(output) /*out*/
            : [Saw]"z"(phase), [Feedback]"z"(feedback) /*in*/
            : /*clobbered*/
            );


    // Write to delay line
    const uint16_t writePos = (readPos + params->delay) & (OSC_FEEDBACK_MAX_DELAY - 1); // TODO mit links/rechtsshift anstelle Wert in zusätzlichem Register
    state->delayLine[writePos] = output;

    // Increment read position
    ++readPos;
    readPos &= (OSC_FEEDBACK_MAX_DELAY - 1); // TODO mit links/rechtsshift anstelle Wert in zusätzlichem Register
    state->readPos = readPos;

    return output;
}

#endif