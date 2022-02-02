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
 * @file lfo_types.h
 * @brief Definition of LFO-related types
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC� digital signal controllers
*/

#ifndef LFO_TYPES_H
#define	LFO_TYPES_H

#include "LFO_enums.h"
#include "fp_lib_types.h"
#include <stdbool.h>

/// LFO state
typedef struct
{
    /// Current LFO phase
    _Q16 phase;

    /// Sync flag indicating an LFO phase overflow
    bool sync;

    /// Current LFO output value (needed for S&H and random)
    _Q15 currentValue;

    /// Last LFO output value (needed for random)
    _Q15 lastValue;
} LFOState;

/// LFO parameters
typedef struct
{
    /// LFO waveform type
    LFOWaveform waveform;

    /// LFO rate
    _Q16 rate;
} LFOParams;

#endif
