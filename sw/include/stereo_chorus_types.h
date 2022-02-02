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
 * @file stereo_chorus_types.h
 * @brief Type definitions for stereo chorus effect
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef STEREO_CHORUS_TYPES_H
#define	STEREO_CHORUS_TYPES_H

#include <stdint.h>
#include "fp_lib_types.h"

/// Stereo chorus parameters
typedef struct
{
    /// Chorus depth 0 ... 255
    uint16_t depth;
    
    /// Chorus modulation rate
    _Q16 rate;
    
    /// Chorus modulation depth
    _Q16 modDepth;
    
    /// Chorus stereo spread
    _Q16 spread;
    
    /// Chorus mix
    _Q15 mix;
} ChorusParams;

#endif

