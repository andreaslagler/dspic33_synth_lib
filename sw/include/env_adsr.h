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
 * @file env_adsr.h
 * @brief Function prototypes for ADSR envelope calculation
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef ENV_ADSR_H
#define	ENV_ADSR_H

#include "env_adsr.h"
#include "env_adsr_types.h"
#include "fp_lib_types.h"
#include "fp_lib_mul.h"
#include <stdint.h>
#include <stdbool.h>

// Forward declaration
extern const _Q16 expDecayTable[];

/**
 * @brief Update ADSR envelope
 * 
 * Update the Envelope state from envelope parameters and return the output value calculated from the updated state
 * @note This envelope is retriggerable
 * @param params Struct holding ADSR envelope parameters
 * @param gate Flag indicating the gate is open
 * @param trigger Flag indicating the envelope has been (re-)triggered
 * @param state Struct holding ADSR envelope state
 * @return Envelope output value in Q0.16 format
 */
static inline _Q16 updateEnvADSR(
        const ADSRParams * const params,
        const bool gate,
        const bool trigger,
        ADSRState * const state)
{
    // Prefetch envelope state
    ADSRStage stage = state->stage;
    _Q16 value = state->value;
    
    // Check which stage the envelope is currently in
    if (!gate)
    {
        // Note off --> Release stage
        stage = ADSR_STAGE_R;
    }
    else if (trigger)
    {
        // (Re-) Trigger the envelope --> Attack stage
        // Since the envelope update is synchronous and a trigger event may
        // occur asynchronously, the trigger flag must be checked explicitly
        // (Checking for rising or falling velocity edges is NOT sufficient)
        stage = ADSR_STAGE_A;
    }

    // Update the envelope value
    switch (stage)
    {
        case ADSR_STAGE_R:
            // Release:
            // Output = LastOutput * R
            value = mul_Q16_Q16(value, expDecayTable[params->release]);
            break;
            // NB. not sure how the compiler translates this switch statement.
            // Since the envelope will be in release state most of the time, it
            // seems to be a good idea to check this condition first.

        case ADSR_STAGE_A:
            // Attack:
            // Output = 1 - (1 - LastOutput) * A
            value = ~value;
            value = mul_Q16_Q16(value, expDecayTable[params->attack]);
            value = ~value;

            // Check for Attack settling. For improvement of the attack shape,
            // the comparison is restricted to the upper word only, since this
            // will translate into the inverse of the output value directly.
            if (value == 0xFFFF)
            {
                // Attack has settled --> Decay
                stage = ADSR_STAGE_D;
            }
            break;

        case ADSR_STAGE_D:
            // Decay:
            // Output = S + (LastOutput - S) * D
            value -= params->sustain;
            value = mul_Q16_Q16(value, expDecayTable[params->decay]);
            value += params->sustain;
            break;

        default:
            value = 0;
            break;
    }

    // Write back envelope state
    state->stage = stage;
    state->value = value;

    return value;
}

#endif
