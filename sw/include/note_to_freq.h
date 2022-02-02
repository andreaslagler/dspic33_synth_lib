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
 * @file note_to_freq.h
 * @brief Function prototypes for note to frequency conversion
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#ifndef NOTE_TO_FREQ_H
#define	NOTE_TO_FREQ_H

#include "note_to_freq.h"
#include "fp_lib_types.h"
#include "fp_lib_mul.h"
#include "stdint.h"
   
extern const _Q32 noteToFreqTable[4097];

/**
 * @brief Convert MIDI note to normalized frequency
 * 
 * @param note Note on MIDI scale in format (semitones * 100 + cents) * 2 (ie. half-cent resolution)
 * @return Normalized frequency of note in Q0.32 format
 */
static inline _Q32 noteToFreq(const int16_t note)
{
    // Fractional note index
    uint16_t noteFrac = note & 0b1111;

    // Integer note index
    const int16_t noteInt = note >> 4; 
    
    // Linear interpolation between two table values
    // Table values are calculated on a grid with spacing 16
    // for a good memory/accuracy tradeoff 
    const _Q32 * freqTable = (noteToFreqTable + 1 + noteInt);
    _Q32 qFreq = mul_Q32_UINT(*freqTable--, noteFrac);
    noteFrac = 16 - noteFrac;
    qFreq += mul_Q32_UINT(*freqTable, noteFrac);

    return qFreq;
}


#endif
