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
 * @file lfo.c
 * @brief Impementation of LFO (Low frequency oscillator)
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#include "lfo.h"
#include "lfo_types.h"
#include "osc_saw.h"
#include "osc_rect.h"
#include "osc_tri.h"
#include "rand.h"
#include "fp_lib_interp.h"
#include "fp_lib_div.h"
#include "fp_lib_trig.h"

// Forward declaration
static const _Q15 lfoRateToFreqTable[];

/**
 * @brief Update the phase of an LFO
 * 
 * Update the LFO phase at selected LFO rate. LFO sync flag is set on a phase overflow
 * @param rate LFO rate in Q0.16 format
 * @param state Struct holding the LFO state
 */
static inline void updateLFOPhase(
        const _Q16 rate,
        LFOState * const state)
{
    // Init current LFO state
    _Q16 phase = state->phase;
    _Q16 sync = 0;

    // Convert LFO rate to frequency
    const _Q15 freq = interpLUT_256_Q15(lfoRateToFreqTable, rate);

    // Increment phase and check for carry
    __asm__ volatile(
            "\
        add     %[phase], %[freq], %[phase]     ;Accumulate phase \n \
        addc    %[sync], %[sync], %[sync]       ;Store carry \n \
        ;2 cycles total"
            : [phase] "+r"(phase), [sync] "+r"(sync) /*out*/
            : [freq] "r"(freq) /*in*/
            : /*clobbered*/
            );

    // Write back LFO state
    state->sync = sync;
    state->phase = phase;
}

/**
 * @brief Calculate LFO value
 * 
 * Calculate LFO output value from LFO state for selected waveform type
 * @param waveform Selected LFO waveform type
 * @param state Struct holding the LFO state
 * @return LFO output value in Q0.15 format
 */
static inline _Q15 calcLFOValue(
        const LFOWaveform waveform,
        LFOState * const state)
{
    // Calculate LFO value for selected waveform
    switch (waveform)
    {
        case ELFO_WAVEFORM_SQUARE:
            // Square
            return calcNaiveRect(state->phase, 0x8000);
            break;

        case ELFO_WAVEFORM_SAW:
            // Sawtooth
            return calcNaiveSaw(state->phase);
            break;

        case ELFO_WAVEFORM_TRI:
            // Triangle
            return calcNaiveTri(state->phase);
            break;

        case ELFO_WAVEFORM_SINE:
            // Sine
            return sin_Q15(state->phase);
            break;

        case ELFO_WAVEFORM_RANDOM:
            // Update current and last values when phase overflows
            if (state->sync)
            {
                state->lastValue = state->currentValue;
                state->currentValue = rand();
            }

            // Output value is interpolated between the last and current value
            return interpLinear(state->lastValue, state->currentValue, state->phase);
            break;

        case ELFO_WAVEFORM_SAMPLEHOLD:
            // Update value when phase overflows
            if (state->sync)
            {
                state->currentValue = rand();
            }

            // Return current value
            return state->currentValue;
            break;

        default:
            // Default, should never be hit
            return 0;
            break;
    }
}

/**
 * @brief Update LFO
 * 
 * Update the LFO state from LFO parameters and return the output value calculated from the updated state
 * @param params Struct holding LFO parameters
 * @param state Struct holding LFO state
 * @return LFO output value in Q0.15 format
 */
_Q15 updateLFO(
        const LFOParams * const params,
        LFOState * const state)
{
    // Update the LFO phase and check for phase overflow
    updateLFOPhase(
            params->rate,
            state);

    // Calculate LFO putput value
    return calcLFOValue(
            params->waveform,
            state);
}

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
        const LFOParams * const params,
        LFOState * const state,
        const _Q16 rateMaster,
        const _Q16 phaseMaster,
        const _Q16 syncPhase)
{
    // Convert LFO rate to frequency
    const _Q16 freqMaster = interpLUT_256_Q15(lfoRateToFreqTable, rateMaster);
    const _Q16 freq = interpLUT_256_Q15(lfoRateToFreqTable, params->rate);

    // Translate master phase into time difference and back into current phase
    // Phase = Freq/FreqMaster * PhaseMaster, truncated to _Q16 (i.e. ignore integer number of cycles)
    const ULong phase = {.value = mul_Q1616_Q16(div_Q16_Q16(freq, freqMaster), phaseMaster)};
    state->phase = phase.low + syncPhase;

    // Calculate LFO putput value
    return calcLFOValue(
            params->waveform,
            state);
}

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
        const LFOWaveform waveform,
        LFOState * const state,
        const _Q16 syncPhase)
{
    // Reset phase to sync phase value
    state->phase = syncPhase;

    // Calculate LFO putput value
    return calcLFOValue(
            waveform,
            state);
}

/**
 * @brief Interpolation table for LFO rate to frequency conversion
 * For calculation of table values see calc_lfo_rate_to_freq_table.m
 */
static const _Q15 lfoRateToFreqTable[257] = {
    1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4,
    4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7,
    7, 8, 8, 8, 9, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12, 13,
    13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 19, 19, 20, 21, 22, 22,
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 35, 36, 37, 39, 40,
    41, 43, 45, 46, 48, 50, 51, 53, 55, 57, 59, 62, 64, 66, 69, 71,
    74, 76, 79, 82, 85, 88, 91, 95, 98, 102, 106, 109, 114, 118, 122, 126,
    131, 136, 141, 146, 151, 157, 163, 169, 175, 181, 188, 195, 202, 209, 217, 225,
    233, 242, 250, 260, 269, 279, 289, 300, 311, 322, 334, 346, 359, 372, 386, 400,
    414, 430, 445, 462, 479, 496, 514, 533, 553, 573, 594, 616, 638, 662, 686, 711,
    737, 764, 792, 821, 851, 882, 915, 948, 983, 1019, 1056, 1095, 1135, 1177, 1220, 1264,
    1311, 1359, 1409, 1460, 1514, 1569, 1627, 1686, 1748, 1812, 1878, 1947, 2018, 2092, 2169, 2248,
    2331, 2416, 2505, 2596, 2692, 2790, 2892, 2998, 3108, 3222, 3340, 3462, 3589, 3721, 3857, 3998,
    4145, 4297, 4454, 4617, 4786, 4962, 5144, 5332, 5527, 5730, 5940, 6157, 6383, 6617, 6859, 7110,
    7371, 7641, 7921, 8211, 8512, 8823, 9147, 9482, 9829, 10189, 10562, 10949, 11350, 11766, 12197, 12644,
    13107
};

