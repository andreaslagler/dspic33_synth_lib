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
 * @file osc_feedback_types.h
 * @brief Definition of feedback oscillator types
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef OSC_FEEDBACK_TYPES_H
#define	OSC_FEEDBACK_TYPES_H

#include "fp_lib_types.h"
#include <stdint.h>

// Length of comb filter delay line (has to be a power of 2)
#define OSC_FEEDBACK_MAX_DELAY_POW2 9
#define OSC_FEEDBACK_MAX_DELAY (1 << OSC_FEEDBACK_MAX_DELAY_POW2)

/// Feedback oscillator parameters
typedef struct
{
    /// Comb filter feedback
    _Q15 feedback;
    
    /// Comb filter delay
    uint16_t delay;
} OscFeedbackParams;

/// Feedback oscillator state
typedef struct
{
    /// Oscillator phase 
    _Q32 phase;

    /// Comb filter delay line
    _Q15 delayLine[OSC_FEEDBACK_MAX_DELAY];
    
    /// Delay line read position
    uint16_t readPos;
} OscFeedbackState;

#endif
