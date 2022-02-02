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
 * @file stereo_delay.h
 * @brief Function prototypes for stereo delay effect
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#ifndef STEREO_DELAY_H
#define	STEREO_DELAY_H

#include "stereo_delay_types.h"
#include "fp_lib_types.h"

/**
 * @brief Add delay to stereo signal
 * @param params Struct holding stereo delay parameters
 * @param state Struct holding stereo delay state
 * @param delayLineLeft Input/output buffer for left delay line
 * @param delayLineRight Input/output buffer for right delay line
 * @param dataLeft Input/output buffer for left stereo channel
 * @param dataRight Input/output buffer for right stereo channel
*/
void addStereoDelay(
        const StereoDelayParams * const params,
        StereoDelayState * const state,
        _Q15 * const delayLineLeft,
        _Q15 * const delayLineRight,
        _Q15 * const dataLeft,
        _Q15 * const dataRight);

#endif