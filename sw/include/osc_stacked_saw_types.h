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
 * @file osc_stacked_saw_types.h
 * @brief Definition of stacked saw oscillator types
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef OSC_STACKED_SAW_TYPES_H
#define	OSC_STACKED_SAW_TYPES_H

#include "fp_lib_types.h"
#include <stdint.h>
#include "svf_2Pole_types.h"

/// Stacked saw oscillator parameters
typedef struct
{
    /// Side oscillator frequencies
    _Q32 freq[6];
    
    /// Center oscillator level
    _Q15 levelCenter;
    
    /// Side oscillator level
    _Q15 levelSide;
    
    /// Anti-aliasing filter coefficients for first filter stage
    _Q15 filterCoeffs1[4];
    
    /// Anti-aliasing filter coefficients for second filter stage
    _Q15 filterCoeffs2[4];
} OscStackedSawParams;

/// Stacked saw oscillator state
typedef struct
{
    /// Center and side oscillator phases
    _Q32 phase[7];
    
    /// Anti-aliasing filter state (first and second stage))
    SVF2PoleState filter[2]; // 4th order Butterworth highpass filter
} OscStackedSawState;

#endif	/* OSC_STACKED_SAW_H */
