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
 * @file rand.h
 * @brief Pseudo-random number generator
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef RAND_H
#define	RAND_H

#include "fp_lib_types.h"

/**
 * @brief Generate a pseudo-random number in Q0.15 format
 * 
 * This function uses a 32-bit random generator as described here: https://www.musicdsp.org/en/latest/Synthesis/216-fast-whitenoise-generator.html
 * @return Pseudo-random number in Q0.15 format
 */
static inline _Q15 rand()
{
    // Static initialization of PN generator state
    static Long state1 = {.value = 0x67452301};
    static Long state2 = {.value = 0xefcdab89};

    // Update PN generator state
    state1.value ^= state2.value;
    state2.value += state1.value;
    
    // Return high word
    return state2.high;
}

#endif
