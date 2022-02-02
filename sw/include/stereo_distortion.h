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
 * @file stereo_distortion.h
 * @brief Impementation of stereo chorus effect
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#ifndef STEREO_DISTORTION_H
#define STEREO_DISTORTION_H

#include "stereo_distortion_types.h"
#include "block_len_def.h"
#include <stdint.h>
#include "fp_lib_types.h"
#include "fp_lib_typeconv.h"
#include "fp_lib_mul.h"

/**
 * @brief Add chorus effect to stereo signal
 * @note The working length of this function is BLOCK_LEN
 * @param params Struct holding stereo chorus parameters
 * @param dataL Audio data for left stereo channel
 * @param dataR Audio data for right stereo channel
 */
static inline void addStereoDistortion(
        const DistortionParams * const params,
        _Q15 * data)
{
    const _Q15 hardShape = mul_Q15_Q15(params->drive, params->shape);
    const _Q15 softShape = params->drive - hardShape;
    
    for (uint16_t cSample = 0; cSample < BLOCK_LEN * 2; ++cSample)
    {
   // Hard clipping
    volatile register int acc asm("A");
    acc = __builtin_mpy(*data, hardShape, 0, 0, 0, 0, 0, 0);
    acc = __builtin_add(acc, *data, 3);
    _Q15 hardClipped = __builtin_sacr(acc, -3);

    // Soft saturation using third order polynomial
    // y = x + 0.5 * (x - x^3) = x - (x * (0.5 * x*x - 0.5))
    _Q15 softClipped = mul_Q15_Q15(hardClipped, hardClipped) >> 1; // 0.5 * x*x
    softClipped -= 16384; // 0.5 * x*x - 0.5
    softClipped = mul_Q15_Q15(hardClipped, softClipped); // x * (0.5 * x*x - 0.5)
    *data++ = hardClipped - mul_Q15_Q15(softClipped, softShape);
    }
}

#endif