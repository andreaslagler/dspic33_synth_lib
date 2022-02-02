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
 * @file osc_colored_noise.h
 * @brief Implementation of colored noise oscillator
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#ifndef OSC_COLORED_NOISE_H
#define	OSC_COLORED_NOISE_H

#include "osc_colored_noise_types.h"
#include "rand.h"
#include "vario_1pole.h"
#include "fp_lib_types.h"
#include <stdint.h>

/**
 * @brief Calculate parameters for colored noise oscillator
 * @param shape The shape parameter in Q0.16 format translates into noise color in Q0.16 format (0 = darkest, 1 = brightest)
 * @param params Struct holding colored noise oscillator parameters
 */
inline static void calcOscColoredNoiseParams(
                                             const _Q16 shape,
                                             OscColoredNoiseParams * const params)
{
    // Noise filter parameters
    calcVario1PoleParams(
                         shape,
                         &params->filterParams);
}

/**
 * @brief Calculate one sample of colored noise oscillator waveform
 * 
 * Update the oscillator state and calculate one waveform sample from the updated state 
 * @param params Struct holding colored noise oscillator parameters
 * @param state Struct holding colored noise oscillator state
 * @return Colored noise oscillator waveform sample in Q0.15 format
 */
inline static _Q15 calcOscColoredNoiseSample(
                                             const OscColoredNoiseParams * const params,
                                             OscColoredNoiseState * const state)
{
    // Get white noise sample
    const _Q15 output = rand();

    // Filter white noise according to color
    return calcVario1PoleSample(
                                &params->filterParams,
                                &state->filterState,
                                output);
}

#endif