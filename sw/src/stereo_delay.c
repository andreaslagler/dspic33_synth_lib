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
 * @file stereo_delay.c
 * @brief Impementation of stereo delay effect
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#include "stereo_delay.h"
#include "stereo_delay_types.h"
#include <stdint.h>
#include "fp_lib_types.h"
#include "block_len_def.h"
#include "vario_1pole.h"

/**
 * @brief Calculate the delay line input from the direct path and delay line signals
 * @param directPath Input/output signal for direct path
 * @param delayLine Input/output signal for delay line
 * @param feedback Feedback amount, i.e. amount of delay line signal which is fed back into the delay line
 * @param mix Dry/Wet mix, i.e. amount of delay line signal which is added to the direct path signal
 */
inline static void calcDelayLineInput(
        _Q15 * directPath,
        _Q15 * delayLine,
        const _Q15 feedback,
        const _Q15 mix)
{
    // temp1 = directPath[k] + delayLine[k] * mix;
    // temp2 = directPath[k] + delayLine[k] * feedback;
    // directPath[k] = temp1
    // delayLine[k] = temp2
    __asm__ volatile(
            "\
        do      #%[len] - 1, calcDelayLineInput_%=  ;Init loop \n \
                                                    ;\n \
        mov     [%[delayLine]], w4                  ;Prefetch delayLine[k] \n \
        mpy     w4 * %[mix], A                      ;AccA = delayLine[k] * mix \n \
        add     [%[directPath]], #0, A              ;AccA += directPath[k] \n \
        mpy     w4 * %[feedback], B                 ;AccB = delayLine[k] * feedback \n \
        add     [%[directPath]], #0, B              ;AccB += directPath[k] \n \
        sac.r   A, #0, [%[directPath]++]            ;directPath[k++] = AccA \n \
                                                    ;\n \
        calcDelayLineInput_%=:                      ;\n \
                                                    ;\n \
        sac.r   B, #0, [%[delayLine]++]             ;delayLine[k++] = AccB \n \
                                                    ;\n \
        ; 2 + 7 * BLOCK_LEN cycles total"
            : [directPath]"+r"(directPath), [delayLine]"+r"(delayLine) /*out*/
            : [len]"i"(BLOCK_LEN), [feedback]"z"(feedback), [mix]"z"(mix) /*in*/
            : "w4" /*clobbered*/
            );
}

/**
 * @brief Add stereo spread to feedback path
 * @param dataLeft Input/output buffer for left stereo channel
 * @param dataRight Input/output buffer for right stereo channel
 * @param spread Stereo spreading factor (0 = no spread ... 1 = stereo inversion/ping-pong)
 */
inline static void addStereoSpread(
        _Q15 * dataLeft,
        _Q15 * dataRight,
        const _Q15 spread)
{
    // temp1 = dataLeft[k] * (1 - spread) + dataRight[k] * spread
    // temp2 = dataRight[k] * (1 - spread) + dataLeft[k] * spread
    // dataLeft[k]  = temp1
    // dataRight[k] = temp2
    __asm__ volatile(
            "\
        do      #%[len] - 1, addStereoSpread_%=     ;Init loop \n \
                                                    ;\n \
        mov     [%[dataLeft]], w4                   ;Prefetch dataLeft[k] \n \
        lac     w4, #0, A                           ;AccA = dataLeft[k] \n \
        msc     w4 * %[spread], A                   ;AccA -= dataLeft[k] * spread => AccA = dataLeft[k] * (1-spread) \n \
        mpy     w4 * %[spread], B                   ;AccB = dataLeft[k] * spread \n \
                                                    ;\n \
        mov     [%[dataRight]], w4                  ;Prefetch dataRight[k] \n \
        mac     w4 * %[spread], A                   ;AccA += dataRight[k] * spread => AccA = dataLeft[k] * (1-spread) + dataRight[k] * spread \n \
        msc     w4 * %[spread], B                   ;AccB -= dataRight[k] * spread => AccB = dataLeft[k] * spread - dataRight[k] * spread \n \
        add     w4, #0, B                           ;AccB += dataRight[k] => AccB = dataLeft[k] * spread + dataRight[k] * (1-spread) \n \
                                                    ;\n \
        sac.r   A, #0, [%[dataLeft]++]              ;dataLeft[k] = AccA \n \
                                                    ;\n \
        addStereoSpread_%=:                         ;\n \
                                                    ;\n \
        sac.r   B, #0, [%[dataRight]++]             ;dataRight[k] = AccB \n \
                                                    ;\n \
        ; 4 + 10 * BLOCK_LEN cycles total"
            : [dataLeft]"+r"(dataLeft), [dataRight]"+r"(dataRight) /*out*/
            : [len]"i"(BLOCK_LEN), [spread]"z"(spread) /*in*/
            : "w4" /*clobbered*/
            );
}

/**
 * @brief Add brightness to stereo signal
 * @param brightness Brightness factor in Q0.16  format (0 = darkest ... 1 = brightest)
 * @param stateLeft brightness filter state for left stereo channel
 * @param stateRight brightness filter state for right stereo channel
 * @param dataLeft Input/output buffer for left stereo channel in Q0.15 format
 * @param dataRight Input/output buffer for right stereo channel in Q0.15 format
 */
inline static void addBrightness(
        const _Q16 brightness,
        IIROnePoleState * const stateLeft,
        IIROnePoleState * const stateRight,
        _Q15 * const dataLeft,
        _Q15 * const dataRight)
{
    Vario1PoleParams params;
    calcVario1PoleParams(
            brightness,
            &params);

    calcVario1PoleStereoBlock(
            &params,
            stateLeft,
            stateRight,
            dataLeft,
            dataRight);
}

/**
 * @brief Add delay to stereo signal
 * @param params Struct holding stereo delay parameters
 * @param state Struct holding stereo delay state
 * @param delayLineLeft Input/output buffer for left delay line
 * @param delayLineRight Input/output buffer for right delay line
 * @param dataLeft Input/output buffer for left stereo channel
 * @param dataRight Input/output buffer for right stereo channel
*/
void addStereoDelay(
        const StereoDelayParams * const params,
        StereoDelayState * const state,
        _Q15 * const delayLineLeft,
        _Q15 * const delayLineRight,
        _Q15 * const dataLeft,
        _Q15 * const dataRight)
{
    // Delay line input for left channel (SRAM0))
    calcDelayLineInput(
            dataLeft,
            delayLineLeft,
            params->feedback,
            params->mix);    

    // Delay line input for right channel (SRAM1)
    calcDelayLineInput(
            dataRight,
            delayLineRight,
            params->feedback,
            params->mix);

    // Add Brightness to feedback signal 
    addBrightness(
            params->brightness,
            &state->filterStateLeft,
            &state->filterStateRight,
            delayLineLeft,
            delayLineRight);

    // Add stereo spread (ping-pong) to feedback signal)
    addStereoSpread(
            delayLineLeft,
            delayLineRight,
            params->spread);
}