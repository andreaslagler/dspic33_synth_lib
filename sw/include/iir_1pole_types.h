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
 * @file iir_1pole_types.h
 * @brief Definition of 1-pole IIR filter types
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef IIR_1POLE_TYPES_H
#define	IIR_1POLE_TYPES_H

#include "fp_lib_types.h"
#include <stdint.h>

/// 1-pole IIR filter parameters
typedef struct
{
    /// Value/mantissa of filter state
    _Q15 stateValue;
    
    /// Scaling factor/exponent of filter state -15 ... +15
    int16_t stateScaling;
} IIROnePoleState;

#endif

