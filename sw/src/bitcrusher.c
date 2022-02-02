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
 * @file stereo_chorus.c
 * @brief Impementation of stereo chorus effect
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#include "bitcrusher.h"
#include "bitcrusher_types.h"
#include "block_len_def.h"
#include <stdint.h>
#include "fp_lib_types.h"
#include "fp_lib_mul.h"
#include "fp_lib_div.h"

void addBitcrusher(
        const BitcrusherParams * const params,
        BitcrusherState * const state,
        _Q15 * dataL,
        _Q15 * dataR)
{
    // Cache state for Read-Write access
    uint16_t clock = state->clock;
    _Q15 lastL = state->lastL;
    _Q15 lastR = state->lastR;

    // Down-scaling value
    const _Q16 downScaling = params->scaling;

    // Up-scaling value = 1 / Down-scaling value
    const _Q1616 upScaling = div_Q16_Q16(32768, downScaling) << 1;

    // Loop all samples
    for (uint16_t cnt = 0; cnt < BLOCK_LEN; ++cnt)
    {
        // Increment clock
        clock += params->sampleRate;
        
        // Check for clock MSB
        if (clock & 32768)
        {
            // Reset clock
            clock &= 32767;
            
            // These are the actually bit-crushed values
            lastL = mul_Q15_Q1616(mul_Q15_Q16(dataL[cnt], downScaling), upScaling);
            lastR = mul_Q15_Q1616(mul_Q15_Q16(dataR[cnt], downScaling), upScaling);
        }

        // Sample and hold of bit-crushed input values
        dataL[cnt] = lastL;
        dataR[cnt] = lastR;
    }
    
    // Write back cached state
    state->clock = clock;
    state->lastL = lastL;
    state->lastR = lastR;
}