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
 * @file env_adsr_types.h
 * @brief Definition of ADSR envelope related types
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef ENV_ADSR_TYPES_H
#define	ENV_ADSR_TYPES_H

#include "fp_lib_types.h"
#include <stdint.h>

/**
 * Stages of an ADSR envelope
 * NB. Sustain is not an actual stage!
 */
typedef enum
{
    ADSR_STAGE_R = 0, // envelope is in the release segment most of the time, so make this condition easy to check against and assign it to integer value 0
    ADSR_STAGE_A,
    ADSR_STAGE_D
} ADSRStage;

/// ADSR envelope state
typedef struct
{
    /// Current ADSR stage
    ADSRStage stage;
    
    /// Current envelope value
    _Q16 value;
} ADSRState;

/// ADSR envelope parameters
typedef struct
{
    /// Attack time 0..255
    uint8_t attack;
    
    /// Decay time 0..255
    uint8_t decay;
    
    /// Sustain level
    _Q16 sustain;
    
    /// Release time 0..255
    uint8_t release;
} ADSRParams;

#endif
