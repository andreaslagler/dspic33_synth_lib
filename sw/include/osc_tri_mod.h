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
 * @file osc_tri_mod.h
 * @brief Implementation of modified triangle waveform oscillator
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef OSC_TRI_MOD_H
#define	OSC_TRI_MOD_H

#include "Osc_Tri.h"
#include "fp_lib_types.h"

/**
 * @brief Calculate one sample of modified triangle oscillator waveform for given oscillator phase
 * 
 * This waveform is a naive triangle waveform modified by a simple scaling+offset wave shaper/folder
 * @param phase Oscillator instantaneous phase in Q0.16 format
 * @param shape1 Scaling factor in Q0.16 format
 * @param shape2 Offset n Q0.16 format
 * @return Modified triangle waveform sample in Q0.15 format
 */
inline static _Q15 calcTriMod(
        const _Q16 phase,
        const _Q16 shape1,
        const _Q16 shape2)
{
    // Output = triangle * (1 + shape1) + shape2
    // NB:
    // int16 overflow is used here intentionally so the triangle wave is folded as desired
    _Q15 output = calcNaiveTri(phase);
    output += mul_Q15_Q16(output, shape1);
    output += shape2;
    
    return output;
}

#endif
