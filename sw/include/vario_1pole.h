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
 * @file variable_1pole.h
 * @brief Implementation of colored noise oscillator
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#ifndef VARIO_1POLE_H
#define	VARIO_1POLE_H

#include "fp_lib_types.h"
#include <stdint.h>
#include "vario_1pole_types.h"
#include "iir_1pole.h"

/**
 * @brief Calculate parameters for colored noise oscillator
 * @param shape The shape parameter in Q0.16 format translates into noise color in Q0.16 format (0 = darkest, 1 = brightest)
 * @param params Struct holding colored noise oscillator parameters
 */
inline static void calcVario1PoleParams(
                                        const _Q16 shape,
                                        Vario1PoleParams * const params)
{
    // Packed representation of noise filter parameters
    const union
    {
        _Q16 shape;
        struct
        {
            uint16_t cutOff : 15;
            uint16_t filterType : 1; // MSB
        };
    } packedParams = {.shape = shape};

    // Unpack noise color
    params->filterType = packedParams.filterType;

    // Calculate filter coefficients for 1-pole filter
    params->alpha = calcIIR1PoleAlpha(packedParams.cutOff << 1);
}

/**
 * @brief Filter one sample by 1-pole IIR variable filter
 * @param shape Filter shape. Lowpass for shape < 0 and highpass for shape > 0
 * @param state Struct holding 1-pole IIR filter struct
 * @param data Block of data samples to be filtered in Q0.15 format
 */
inline static void calcVario1PoleBlock(
                                       const Vario1PoleParams * const params,
                                       IIROnePoleState * const state,
                                       _Q15 * const data)
{
    typedef void (*CalcFilter1PoleBlock)(const _Q15, IIROnePoleState * const, _Q15 *);
    static const CalcFilter1PoleBlock calcFilter1PoleBlock[2] = {
        calcLP1PoleBlock,
        calcHP1PoleBlock
    };

    calcFilter1PoleBlock[params->filterType](params->alpha, state, data);
}

/**
 * @brief Filter one sample by 1-pole IIR variable filter
 * @param shape Filter shape. Lowpass for shape < 0 and highpass for shape > 0
 * @param stateLeft Struct holding 1-pole IIR filter struct for left stereo channel
 * @param stateRight Struct holding 1-pole IIR filter struct for right stereo channel
 * @param dataLeft Block of data samples to be filtered for left stereo channel in Q0.15 format
 * @param dataRight Block of data samples to be filtered for right stereo channel in Q0.15 format
 */
inline static void calcVario1PoleStereoBlock(
                                             const Vario1PoleParams * const params,
                                             IIROnePoleState * const stateLeft,
                                             IIROnePoleState * const stateRight,
                                             _Q15 * const dataLeft,
                                             _Q15 * const dataRight)
{
    typedef void (*CalcFilter1PoleBlock)(const _Q15, IIROnePoleState * const, _Q15 *);
    static const CalcFilter1PoleBlock calcFilter1PoleBlock[2] = {
        calcLP1PoleBlock,
        calcHP1PoleBlock
    };
    const CalcFilter1PoleBlock calcFilter = calcFilter1PoleBlock[params->filterType];
    calcFilter(params->alpha, stateLeft, dataLeft);
    calcFilter(params->alpha, stateRight, dataRight);
}

/**
 * @brief Filter one sample by 1-pole IIR variable filter
 * @param shape Filter shape. Lowpass for shape < 0 and highpass for shape > 0
 * @param state Struct holding 1-pole IIR filter struct
 * @param data Block of data samples to be filtered in Q0.15 format
 */
inline static _Q15 calcVario1PoleSample(
                                        const Vario1PoleParams * const params,
                                        IIROnePoleState * const state,
                                        const _Q15 data)
{
    typedef _Q15(*CalcFilter1PoleSample)(const _Q15, IIROnePoleState * const, const _Q15);

    // Jump table for low/high pass
    static const CalcFilter1PoleSample calcFilter1PoleSample[2] = {
        calcLP1PoleSample,
        calcHP1PoleSample
    };

    return calcFilter1PoleSample[params->filterType](params->alpha, state, data);
}

#endif