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
 * @file iir_1pole.c
 * @brief Implementation of 1-pole IIR filter
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#include "fp_lib_types.h"

// Lookup table for filter coefficient
// g(note) = tan(pi * freq(note) / sampleRate)
// For calculation of table values see calc_note_to_svf_g_table.m
const _Q15 noteToSVF2PoleGTable[257] = {
    2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4,
    4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7,
    7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 12, 12, 12,
    13, 13, 14, 14, 15, 16, 16, 17, 17, 18, 19, 19, 20, 21, 22, 23,
    23, 24, 25, 26, 27, 28, 29, 30, 31, 33, 34, 35, 36, 38, 39, 41,
    42, 44, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 66, 68, 71, 73,
    76, 79, 82, 85, 88, 92, 95, 99, 102, 106, 110, 114, 119, 123, 128, 133,
    138, 143, 148, 154, 160, 166, 172, 178, 185, 192, 199, 207, 215, 223, 231, 240,
    249, 259, 268, 278, 289, 300, 311, 323, 335, 348, 361, 375, 389, 404, 419, 435,
    451, 468, 486, 505, 524, 544, 565, 586, 609, 632, 656, 681, 707, 735, 763, 792,
    823, 855, 888, 922, 958, 996, 1035, 1076, 1118, 1162, 1209, 1257, 1307, 1360, 1415, 1473,
    1533, 1597, 1663, 1733, 1806, 1883, 1964, 2050, 2140, 2235, 2336, 2444, 2558, 2679, 2808, 2947,
    3095, 3255, 3428, 3614, 3818, 4040, 4284, 4554, 4853, 5189, 5568, 6000, 6499, 7082, 7489, 7489,
    7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489,
    7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489,
    7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489, 7489,
    7489
};