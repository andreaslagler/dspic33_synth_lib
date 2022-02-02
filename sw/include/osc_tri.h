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
 * @file osc_tri.h
 * @brief Implementation of triangle waveform oscillator
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef OSC_TRI_H
#define	OSC_TRI_H

#include "fp_lib_types.h"
#include "fp_lib_typeconv.h"
#include "fp_lib_abs.h"
#include "fp_lib_mul.h"

/**
 * @brief Calculate one sample of naive triangle oscillator waveform for given oscillator phase
 * @param phase Oscillator instantaneous phase in Q0.16 format
 * @return Saw waveform sample in Q0.15 format
 */
static inline _Q15 calcNaiveTri(const _Q16 phase)
{
    // triangle = 2 * (abs(phase + 0.5) - 0.5);
    _Q15 result = abs_Q15((_Q15) (phase + 16384));
    result -= 16384;
    result <<= 1;
    return result;
}

/**
 * @brief Calculate parameters for triangle waveform oscillator
 * @param shape The shape parameter in Q0.16 format translates into a mixing ratio of triangle and sine waveform components
 * @return Scaling factor for triangle and sine waveform components in Q0.15 format
 */
inline static _Q15 calcTriOscShape(const _Q16 shape)
{
    // For triangle waveform, the shape parameter translates into a scaling parameter for mixing the triangle and sine components of the resulting waveform
    // scaling = 0.125 / (0.125 + (1 - 0.125) * Shape^2)
    // Shape = 0 .. 1 ==> scaling = 1 .. 0.125
    return __builtin_divf(4096, 4097 + convert_Q16_Q15(mul_Q16_Q16(57343, mul_Q16_Q16(shape, shape))));
}

/**
 * @brief Calculate one sample of triangle oscillator waveform for given oscillator phase
 * 
 * This waveform is a mix of a naive triangle waveform and its fundamental sine waveform
 * @param phase Oscillator instantaneous phase in Q0.16 format
 * @param shape Scaling factor for triangle and sine waveform components in Q0.15 format
 * @return Triangle waveform sample in Q0.15 format
 */
inline static _Q15 calcOscTri(
                              const _Q16 phase,
                              const _Q15 shape)
{
    // Calculate triangle and sine components of the waveform
    const _Q15 sine = sin_Q15(phase);
    const _Q15 tri = calcNaiveTri(phase);

    // Calculate the weighted sum of saw and sine component
    _Q15 result = 0;
    __asm__ volatile(
            "\
        lac     %[sine], #3, A                                                  ;AccA = sine * 0.125 \n \
        msc     %[sine] * %[shape], A                                         ;AccA -= sine * scaling factor \n \
        mac     %[tri] * %[shape], A                                          ;AccA += tri * scaling factor \n \
        sac.r   A, #-3, %[result]                                               ;Out = AccA * 8 \n \
                                                                                ;\n \
        ; 4 cycles total"
            : [result]"=r"(result) /*out*/
            : [sine]"z"(sine), [tri]"z"(tri), [shape]"z"(shape) /*in*/
            : /*clobbered*/
            );

    return result;
}

#endif
