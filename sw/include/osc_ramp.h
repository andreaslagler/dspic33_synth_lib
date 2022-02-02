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
 * @file osc_ramp.h
 * @brief Implementation of ramp oscillator
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef OSC_RAMP_H
#define	OSC_RAMP_H

#include "fp_lib_types.h"
#include <stdint.h>

/**
 * @brief Calculate one sample of naive ramp oscillator waveform for given oscillator phase
 * @param phase Oscillator instantaneous phase in Q0.16 format
 * @return Ramp waveform sample in Q0.15 format
 */
static inline _Q15 calcNaiveRamp(_Q16 phase)
{
    // Oscillator cycle starting at -1
    __builtin_btg(&phase, 15);
    
    return (_Q15)phase;
}

#endif
