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
 * @file osc_saw.h
 * @brief Implementation of saw waveform oscillator
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef OSC_SAW_H
#define	OSC_SAW_H

#include "fp_lib_types.h"
#include "fp_lib_trig.h"

/**
 * @brief Calculate one sample of naive saw oscillator waveform for given oscillator phase
 * @param phase Oscillator instantaneous phase in Q0.16 format
 * @return Saw waveform sample in Q0.15 format
 */
inline static _Q15 calcNaiveSaw(const _Q16 phase)
{
    return (_Q15) (phase);
}

/**
 * @brief Calculate parameters for saw waveform oscillator
 * @param shape The shape parameter in Q0.16 format translates into a mixing ratio of saw and sine waveform components
 * @param scaling Scaling factors for saw and sine waveform components in Q0.15 format
 */
inline static void calcOscSawShape(
                                   _Q16 shape,
                                   _Q15 * scaling)
{
    // NB:
    // - Use DSP instructions to get saturated values instead of overflow
    // - Variable "scaling" points to an array of at least two elements
    __asm__ volatile(
            "\
        btg     %[shape], #15                                                   ;Change zero position of the unipolar shape parameter (value range 0..1) to a bipolar Scale parameter (value range -1..1) by toggling the MSB \n \
        mov     #12943, w4                                                      ;w4 = 0.5 scaled by an additional global factor of 0.79 to avoid clipping \n \
        lac     w4, #0, A                                                       ;AccA = 0.5 * 0.79 \n \
        mov     #25887, w4                                                      ;w4 = global factor of 0.79 to avoid clipping \n \
        mac     w4 * %[shape], A                                                ;AccA += 0.79 * shape, AccA = Scaling factor of sine amount \n \
        sac.r   A, #0, [%[scaling]++]                                          ;Store Scaling factor of sine amount \n \
        msc     w4 * %[shape], A                                                ;AccA -= 0.79 * shape \n \
        msc     w4 * %[shape], A                                                ;AccA -= 0.79 * shape, AccA = Scaling factor of saw amount \n \
        sac.r   A, #0, [%[scaling]--]                                          ;Store Scaling factor of saw amount \n \
                                                                                ;\n \
        ; 9 cycles total"
            : [shape]"+z"(shape), [scaling]"+r"(scaling) /*out*/
            : /*in*/
            : "w4" /*clobbered*/
            );
}

/**
 * @brief Calculate one sample of saw oscillator waveform for given oscillator phase
 * 
 * This waveform is a mix of a naive saw waveform and its fundamental sine waveform
 * @param phase Oscillator instantaneous phase in Q0.16 format
 * @param scaling Scaling factors for saw and sine waveform components in Q0.15 format
 * @return Saw waveform sample in Q0.15 format
 */
inline static _Q15 calcOscSaw(
                           const _Q16 phase,
                           _Q15 * scaling) // Has to be allocated in .ydata // static _Q15 aqScaling[2] __attribute__((space(ymemory), far)); 
{
    // Calculate saw and sine components of the waveform
    const _Q15 sine = sin_Q15(phase);
    const _Q15 saw = calcNaiveSaw(phase);

    // Calculate the weighted sum of saw and sine component
    _Q15 output = 0;
    __asm__ volatile(
            "\
        clr     A, [%[scaling]]+=2, w4                                         ;AccA = 0 \n \
        mac     w4 * %[sine], A, [%[scaling]]-=2, w4                          ;AccA += sine * sine scaling factor \n \
        mac     w4 * %[saw], A                                                 ;AccA += saw * saw scaling factor \n \
        sac.r   A, #0, %[output]                                                ;Out = AccB \n \
                                                                                ;\n \
        ; 4 cycles total"
            : [output]"=r"(output), [scaling]"+y"(scaling) /*out*/
            : [sine]"z"(sine), [saw]"z"(saw) /*in*/
            : "w4" /*clobbered*/
            );

    return output;
}
#endif	/* VCO_SAW_H */

