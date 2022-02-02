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
 * @file osc_colored_noise_types.h
 * @brief Definition of colored noise oscillator types
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef OSC_COLORED_NOISE_TYPES_H
#define	OSC_COLORED_NOISE_TYPES_H

#include "fp_lib_types.h"
#include "vario_1pole_types.h"
#include "IIR_1Pole_types.h"

/// Colored noise oscillator parameters
typedef struct
{
    /// Filter parameter
    Vario1PoleParams filterParams;
} OscColoredNoiseParams;

/// Colored noise oscillator state
typedef struct
{
    /// Oscillator phase 
    _Q32 phase;
    
    /// Filter state
    IIROnePoleState filterState;
} OscColoredNoiseState;

#endif