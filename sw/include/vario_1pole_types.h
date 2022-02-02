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
 * @file variable_1pole_types.h
 * @brief Implementation of variable 1-pole IIR filter
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef VARIO_1POLE_TYPES_H
#define	VARIO_1POLE_TYPES_H

#include "fp_lib_types.h"
#include <stdint.h>

/// Colored noise oscillator parameters
typedef struct
{
    /// Filter type
    // 0 - Low pass
    // 1 - High pass
    uint16_t filterType;
    
    /// Filter parameter
    _Q15 alpha;
} Vario1PoleParams;


#endif