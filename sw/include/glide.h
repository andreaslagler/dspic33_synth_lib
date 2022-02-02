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
 * @file glide.h
 * @brief Function prototypes for glide
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef GLIDE_H
#define	GLIDE_H

#include "glide_types.h"
#include "fp_lib_types.h"
#include <stdint.h>

/**
 * @brief Update Glide
 * 
 * Update the Glide state from glide transition parameters and return the output value calculated from the updated state
 * @param params Struct holding ADSR envelope parameters
 * @param state Struct holding ADSR envelope state
 * @return Glide output value in half-cent format
 */
int16_t updateGlide(
        const GlideParams * const params,
        GlideState * const state);

#endif
