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
 * @file tone_control_2bands.h
 * @brief Function prototypes for 2-band tone control
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#ifndef TONE_CONTROL_2BANDS_H
#define	TONE_CONTROL_2BANDS_H

#include "fp_lib_types.h"
#include "tone_control_2band_types.h"

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
        _Q15 * const bufferRight);


#endif

