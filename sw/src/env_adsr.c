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
 * @file env_adsr.c
 * @brief Impementation of ADSR envelope
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
*/

#include "fp_lib_types.h"


/**
 * @brief exponential decay factor
 * 
 * Envelope shape is given by exponential decay which is calculated by continuous multiplication with a decay factor alpha:\n
 * y(n) = alpha ^ n = exp(n * ln(alpha))\n
 * For calculation of table values see calc_exp_decay_table.m
 */
const _Q16 expDecayTable[256] = {
    8869, 9571, 10299, 11051, 11827, 12624, 13441, 14277, 15131, 16000, 16883, 17778, 18684, 19598, 20521, 21449,
    22381, 23316, 24252, 25188, 26123, 27054, 27981, 28903, 29819, 30727, 31626, 32516, 33396, 34265, 35122, 35967,
    36798, 37616, 38420, 39210, 39984, 40744, 41488, 42217, 42930, 43628, 44309, 44975, 45625, 46258, 46876, 47478,
    48065, 48636, 49192, 49732, 50258, 50769, 51265, 51747, 52214, 52668, 53109, 53536, 53950, 54351, 54740, 55117,
    55481, 55834, 56176, 56507, 56827, 57136, 57436, 57725, 58005, 58275, 58536, 58789, 59033, 59268, 59496, 59715,
    59927, 60132, 60329, 60520, 60703, 60881, 61052, 61217, 61376, 61530, 61678, 61820, 61958, 62091, 62219, 62342,
    62461, 62575, 62685, 62792, 62894, 62993, 63088, 63179, 63268, 63352, 63434, 63513, 63589, 63662, 63732, 63800,
    63866, 63928, 63989, 64047, 64103, 64157, 64209, 64259, 64307, 64354, 64398, 64441, 64483, 64523, 64561, 64598,
    64633, 64667, 64700, 64732, 64762, 64792, 64820, 64847, 64873, 64898, 64922, 64946, 64968, 64990, 65010, 65030,
    65050, 65068, 65086, 65103, 65119, 65135, 65150, 65165, 65179, 65193, 65206, 65218, 65230, 65242, 65253, 65264,
    65274, 65284, 65294, 65303, 65312, 65320, 65329, 65336, 65344, 65351, 65358, 65365, 65372, 65378, 65384, 65390,
    65395, 65401, 65406, 65411, 65415, 65420, 65424, 65429, 65433, 65437, 65441, 65444, 65448, 65451, 65454, 65457,
    65460, 65463, 65466, 65469, 65471, 65474, 65476, 65478, 65481, 65483, 65485, 65487, 65489, 65490, 65492, 65494,
    65495, 65497, 65498, 65500, 65501, 65503, 65504, 65505, 65506, 65507, 65508, 65509, 65510, 65511, 65512, 65513,
    65514, 65515, 65516, 65517, 65517, 65518, 65519, 65519, 65520, 65521, 65521, 65522, 65522, 65523, 65523, 65524,
    65524, 65525, 65525, 65526, 65526, 65526, 65527, 65527, 65527, 65528, 65528, 65528, 65529, 65529, 65529, 65529
};