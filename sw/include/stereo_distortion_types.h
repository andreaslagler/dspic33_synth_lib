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
 * @file stereo_distortion_types.h
 * @brief Impementation of stereo chorus effect
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#ifndef STEREO_DISTORTION_TYPES_H
#define STEREO_DISTORTION_TYPES_H

#include "fp_lib_types.h"

/// Distortion parameters
typedef struct
{
    /// Drive parameter
    _Q15 drive;
    
    /// Shape of distortion curve (0 = soft ... 1 = hard)
    _Q15 shape;
    
    _Q16 mix;
} DistortionParams;

#endif