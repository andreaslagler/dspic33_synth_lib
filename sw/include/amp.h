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

#ifndef AMP_H
#define	AMP_H

#include "amp_types.h"
#include "fp_lib_types.h"
#include "fp_lib_typeconv.h"
#include "fp_lib_mul.h"
#include "block_len_def.h"


inline static void calcAmp(
                           const AmpParams * const params,
                           const _Q15 * const input,
                           _Q15 * const outputLeft,
                           _Q15 * const outputRight)
{
    // VCA (left channel)

    _Q15 pan = params->pan;

    // Calculate gain for right channel
    // gain = 1 - 0   for Pan < 0
    //      = 1 - Pan for Pan >= 0
    _Q15 gainL = (pan & (~(pan >> 15)));
    gainL = mul_Q16_Q16(params->gain, ~convert_Q15_Q16_Naive(gainL));

    mul_aQ15_Q16(
            input,
            gainL,
            outputLeft,
            BLOCK_LEN);

    // VCA (right channel)

    // Invert Pan value for right channel
    pan = ~pan;

    // Calculate gain for right channel
    // gain = 1   for Pan < 0
    //      = 1 - Pan for Pan >= 0

    _Q15 gainR = (pan & (~(pan >> 15)));
    gainR = mul_Q16_Q16(params->gain, ~convert_Q15_Q16_Naive(gainR));

    mul_aQ15_Q16(
            input,
            gainR,
            outputRight,
            BLOCK_LEN);

    
}

#endif
