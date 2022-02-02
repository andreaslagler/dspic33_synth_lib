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
 * @file osc_lowpass_noise_types.h
 * @brief Definition of lowpass noise oscillator types
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef OSC_LOWPASS_NOISE_TYPES_H
#define	OSC_LOWPASS_NOISE_TYPES_H

#include "fp_lib_types.h"
#include "svf_2pole_types.h"

/// Lowpass noise oscillator parameters
typedef struct
{
    /// Filter coefficients
    _Q15 filterCoeffs[4];
} OscLowPassNoiseParams;

/// Lowpass noise oscillator state
typedef struct
{
    /// Oscillator phase
    _Q32 phase;
    
    /// Filter state
    SVF2PoleState filter;
} OscLowPassNoiseState;

#endif