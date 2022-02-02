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
 * @file lfo.h
 * @brief Function prototypes for LFO (Low frequency oscillator) calculation
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef LFO_H
#define	LFO_H

#include "lfo_types.h"
#include "fp_lib_types.h"

/**
 * @brief Update LFO
 * 
 * Update the LFO state from LFO parameters and return the output value calculated from the updated state
 * @param params Struct holding LFO parameters
 * @param state Struct holding LFO state
 * @return LFO output value in Q0.15 format
 */
_Q15 updateLFO(
        const LFOParams * params,
        LFOState * state);

/**
 * @brief Update LFO in hard-sync mode
 * 
 * Update the LFO state from LFO parameters and return the output value calculated from the updated state.
 * The phase is hard-synced to a master LFO and reset to selected sync phase
 * @param params Struct holding LFO parameters
 * @param state Struct holding LFO state
 * @param rateMaster Rate of master LFO
 * @param phaseMaster Phase of master LFO
 * @param syncPhase Initial phase after sync occurs
 * @return LFO output value in Q0.15 format
 */
_Q15 updateLFOSync(
        const LFOParams * params,
        LFOState * state,
        _Q16 rateMaster,
        _Q16 phaseMaster,
        _Q16 syncPhase);

/**
 * @brief Update LFO in hard-reset mode
 * 
 * Update the LFO state from LFO parameters and return the output value calculated from the updated state.
 * The phase is hard-reset to selected sync phase
 * @param waveform Selected LFO waveform type
 * @param state Struct holding LFO state
 * @param syncPhase Initial phase after sync occurs
 * @return LFO output value in Q0.15 format 
 */
_Q15 updateLFOReset(
        LFOWaveform waveform,
        LFOState * state,
        _Q16 syncPhase);

#endif
