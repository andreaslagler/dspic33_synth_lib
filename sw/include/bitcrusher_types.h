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
 * @file bitcrusher_types.h
 * @brief Type definitions for stereo bitcrusher effect
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef BITCRUSHER_TYPES_H
#define	BITCRUSHER_TYPES_H

#include <stdint.h>
#include "fp_lib_types.h"

/// Bitcrusher state
typedef struct
{
    /// Bitcrusher clock
    uint32_t clock;
    
    /// Last sample on left channel
    _Q15 lastL;
        
    /// Last sample on right channel
    _Q15 lastR;
} BitcrusherState;

/// Bitcrusher parameters
typedef struct
{
    /// Bitcrusher sample rate
    _Q16 sampleRate;
    
    /// Bitcrusher quantization
    _Q16 quantization;
        
    /// Bitcrusher mix
    _Q15 mix;
} BitcrusherParams;

#endif

