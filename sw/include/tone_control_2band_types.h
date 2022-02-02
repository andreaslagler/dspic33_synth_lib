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
 * @file tone_control_types.h
 * @brief Definition of 2-pole state-variable filter (SVF) types
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef TONE_CONTROL_2BAND_TYPES_H
#define	TONE_CONTROL_2BAND_TYPES_H

#include "fp_lib_types.h"

/// Tone control state
typedef struct
{
    /// Bass filter state for left stereo channel
    _Q15 bassStateLeft[2];

    /// Bass filter state for right stereo channel
    _Q15 bassStateRight[2];
    
    /// Treble filter state for left stereo channel
    _Q15 trebleStateLeft[2];
    
    /// Treble filter state for right stereo channel
    _Q15 trebleStateRight[2];
} ToneControl2BandState;


/// Tone control state
typedef struct
{
    /// Bass parameter
    _Q15 bass;
    
    /// Treble parameter
    _Q15 treble;
} ToneControl2BandParams;

#endif