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
 * @file osc_rect.h
 * @brief Implementation of rectangle wave oscillator
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef OSC_RECT_H
#define	OSC_RECT_H

#include "fp_lib_types.h"

/**
 * @brief Calculate one sample of naive rectangle oscillator waveform for given oscillator phase and pulsewidth
 * 
 * Rectangle waveform =\n
 * 0x7FFF if phase < Pulse width\n
 * 0x8000 if phase > Pulse width
 * @param phase Oscillator instantaneous phase in Q0.16 format
 * @param pulseWidth Puls width in Q0.16 format
 * @return Rectangle waveform sample in Q0.15 format
 */
static inline _Q15 calcNaiveRect(
                                 const _Q16 phase,
                                 const _Q16 pulseWidth)
{
    _Q15 result;

    __asm__ volatile(
            "\
        cp      %[phase], %[pulseWidth]         ;Carry is set if phase < pulseWidth \n \
        rrc     %[result], %[result]            ;Rotate carry into result<15>. result is 0x8000 if phase < pulseWidth or 0 if phase > pulseWidth \n \
        asr     %[result], #15, %[result]       ;Fill result with sign bit using arithmetic right-shift. result is 0xFFFF if phase < pulseWidth or 0 if phase > pulseWidth \n \
        btg     %[result], #15                  ;Toggle sign bit. result is 0x7FFF if phase < pulseWidth or 0x8000 if phase > pulseWidth \n \
        ;4 cycles total"
            : [result] "=r"(result) /*out*/
            : [phase] "r"(phase), [pulseWidth] "r"(pulseWidth)/*in*/
            : /*clobbered*/
            );

    return result;
}

#endif
