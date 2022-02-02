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
// alpha = exp(-2* pi * f0);
// For calculation of table values see calc_note_to_iir_1pole_alpha_table.m
const _Q15 noteToIIR1PoleAlphaTable[257] = {
    32733, 32732, 32730, 32729, 32727, 32726, 32724, 32723, 32721, 32719, 32717, 32715, 32713, 32711, 32709, 32707,
    32705, 32702, 32700, 32697, 32695, 32692, 32689, 32686, 32683, 32680, 32676, 32673, 32669, 32666, 32662, 32658,
    32654, 32649, 32645, 32640, 32636, 32631, 32625, 32620, 32615, 32609, 32603, 32597, 32590, 32583, 32576, 32569,
    32562, 32554, 32546, 32538, 32529, 32520, 32511, 32501, 32491, 32481, 32470, 32459, 32447, 32435, 32423, 32410,
    32396, 32383, 32368, 32353, 32338, 32322, 32305, 32288, 32270, 32251, 32232, 32212, 32191, 32169, 32147, 32124,
    32100, 32075, 32049, 32022, 31995, 31966, 31936, 31905, 31873, 31840, 31805, 31770, 31733, 31694, 31655, 31614,
    31571, 31527, 31481, 31433, 31384, 31333, 31280, 31226, 31169, 31110, 31050, 30987, 30922, 30854, 30784, 30712,
    30637, 30560, 30479, 30396, 30311, 30222, 30130, 30035, 29936, 29835, 29729, 29621, 29508, 29392, 29272, 29148,
    29020, 28887, 28750, 28609, 28463, 28313, 28157, 27997, 27832, 27661, 27485, 27304, 27117, 26924, 26726, 26521,
    26311, 26094, 25871, 25642, 25406, 25164, 24915, 24659, 24397, 24127, 23851, 23567, 23276, 22979, 22674, 22361,
    22042, 21715, 21381, 21040, 20692, 20337, 19975, 19606, 19230, 18848, 18460, 18065, 17664, 17258, 16846, 16430,
    16008, 15582, 15152, 14718, 14281, 13841, 13399, 12955, 12510, 12065, 11619, 11174, 10731, 10289, 9850, 9414,
    8982, 8555, 8133, 7717, 7308, 6906, 6513, 6129, 5754, 5389, 5035, 4692, 4361, 4042, 3735, 3442,
    3162, 2895, 2643, 2404, 2178, 1967, 1769, 1585, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416,
    1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416,
    1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416, 1416,
    1416
};
