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
 * @file stereo_chorus.c
 * @brief Impementation of stereo chorus effect
 * 
 * This file is part of Synth-Lib fixed-point synthesizer library for dsPIC33 family of Microchip dsPIC® digital signal controllers
 */

#include "stereo_chorus.h"
#include "lfo.h"
#include "block_len_def.h"
#include <stdint.h>
#include "fp_lib_types.h"
#include "fp_lib_mul.h"


// 96 * 16 = 256 * 2 * 3 Samples
// 768 +/- 768 delay corresponds to 0..32 ms (@ 48 kHz)
#define NOF_BLOCKS_POW2 4
#define NOF_BLOCKS (1<<NOF_BLOCKS_POW2)
#define RINGBUFFER_SIZE (NOF_BLOCKS * BLOCK_LEN)

// Bitmask for block modulo addressing
#define BLOCK_ADDR_BITMASK (NOF_BLOCKS-1)

// Depth factor. This is the one-sided maximum delay to be multiplied by an 8-bit unsigned depth value
#define DEPTH_FACTOR (BLOCK_LEN >> (9-NOF_BLOCKS_POW2))

/**
 * @brief Copy one block of BLOCK_LEN values of type int16_t from source to destination buffer
 * @param src Pointer to source buffer
 * @param dst Pointer to destination buffer
 */
inline static void copyBlock(
        const int16_t * src,
        int16_t * dst)
{
    // Copy one block 
    __asm__ volatile(
            "\
        repeat  #%[len]-1                  ;\n \
        mov     [%[src]++], [%[dst]++]     ;\n \
        ; 1 + len cycles total"
            : [dst] "+r"(dst), [src] "+r"(src) /*out*/
            : [len] "i"(BLOCK_LEN) /*in*/
            : /*clobbered*/
            );
}

/**
 * @brief Read one block of data from the delay line at given position and mix to signal
 * @note The working length of this function is BLOCK_LEN
 * @param delayLine Pointer to delay line buffer
 * @param delayLineReadPos Current read position within delay line
 * @param mix Dry/Wet mix amount
 * @param data Signal to mix the chorus output to
 */
static inline void addDelayLineOutput(
        _Q15 * const delayLine,
        uint16_t delayLineReadPos,
        const _Q15 mix,
        _Q15 * data)
{
    // Wrap around read pos
    // We avoid a while () loop here and make sure read pos is not out of range)
    if (delayLineReadPos >= RINGBUFFER_SIZE)
        delayLineReadPos += RINGBUFFER_SIZE;

    // Read output data from ring buffer
    const uint16_t nofSamples = RINGBUFFER_SIZE - delayLineReadPos;
    if (nofSamples >= BLOCK_LEN)
    {
        // Output data can be read in one go
        _Q15 * pqRead = delayLine + delayLineReadPos;

        // TODO DSP prefetch statt mov
        __asm__ volatile(
                "\
        do      #%[Len]-1, ReadDelayLine_1_%=                                   ;2C \n \
        mov     [%[In]++], w4                                                   ;1C \n \
        mpy     w4 * %[Scale], A                                                ;1C AccA = Scale * In[k] \n \
        add     [%[InOut]], #0, A                                               ;1C AccA += InOut[k] \n \
        ReadDelayLine_1_%=:                                                     ;\n \
        sac.r   A, #0, [%[InOut]++]                                             ;1C InOut[k++] = AccA \n \
                                                                                ;\n \
        ; 2 + 4N cycles total"
                : [In]"+r"(pqRead), [InOut]"+r"(data) /*out*/
                : [Scale]"z"(mix), [Len]"i"(BLOCK_LEN) /*in*/
                : "w4" /*clobbered*/
                );
    }
    else
    {
        // Read first part of the output data
        _Q15 * pqRead = delayLine + delayLineReadPos;

        // TODO DSP prefetch statt mov
        __asm__ volatile(
                "\
        do      %[Len], ReadDelayLine_2_%=                                      ;2C \n \
        mov     [%[In]++], w4                                                   ;1C \n \
        mpy     w4 * %[Scale], A                                                ;1C AccA = Scale * In[k] \n \
        add     [%[InOut]], #0, A                                               ;1C AccA += InOut[k] \n \
        ReadDelayLine_2_%=:                                                          ;\n \
        sac.r   A, #0, [%[InOut]++]                                             ;1C InOut[k++] = AccA \n \
                                                                               ;\n \
        ; 2 + 4N cycles total"
                : [In]"+r"(pqRead), [InOut]"+r"(data) /*out*/
                : [Scale]"z"(mix), [Len]"r"(nofSamples - 1) /*in*/
                : "w4" /*clobbered*/
                );

        // Read second part of the output data
        pqRead = delayLine; // Second part starts with first sample of ring buffer; output pointer is already set

        __asm__ volatile(
                "\
        do      %[Len], ReadDelayLine_3_%=                                      ;2C \n \
        mov     [%[In]++], w4                                                   ;1C \n \
        mpy     w4 * %[Scale], A                                                ;1C AccA = Scale * In[k] \n \
        add     [%[InOut]], #0, A                                               ;1C AccA += InOut[k] \n \
        ReadDelayLine_3_%=:                                                          ;\n \
        sac.r   A, #0, [%[InOut]++]                                             ;1C InOut[k++] = AccA \n \
                                                                                ;\n \
        ; 2 + 4N cycles total"
                : [In]"+r"(pqRead), [InOut]"+r"(data) /*out*/
                : [Scale]"z"(mix), [Len]"r"(BLOCK_LEN - nofSamples - 1) /*in*/
                : "w4" /*clobbered*/
                );

    }

    data -= BLOCK_LEN;
}

/**
 * @brief Add chorus effect to stereo signal
 * @note The working length of this function is BLOCK_LEN
 * @param params Struct holding stereo chorus parameters
 * @param dataL Audio data for left stereo channel
 * @param dataR Audio data for right stereo channel
 */
void addStereoChorus(
        const ChorusParams * const params,
        _Q15 * dataL,
        _Q15 * dataR)
{
    // Step 1: Feed delay lines
    static _Q15 ringBufferL[RINGBUFFER_SIZE]; // TODO in chorus state verschieben
    static _Q15 ringBufferR[RINGBUFFER_SIZE]; // TODO in chorus state verschieben

    // Increment and roll over write position
    static uint16_t blockWritePos = BLOCK_ADDR_BITMASK; // TODO in chorus state verschieben
    blockWritePos++;
    blockWritePos &= BLOCK_ADDR_BITMASK;

    // Write input data to ring buffer
    // This can always be done in one go, because the ring buffer size is an integer multiple of the block size
    const uint16_t sampleWritePos = blockWritePos * BLOCK_LEN;
    copyBlock(dataL, &ringBufferL[sampleWritePos]);
    copyBlock(dataR, &ringBufferR[sampleWritePos]);


    ////////////////////////////////////////////////////////////////////////////
    // Calc current delay from LFO
    static LFOState lfoState; // No need to init, TODO in chorus state verschieben
    const LFOParams sLFOParams = {.waveform = ELFO_WAVEFORM_RANDOM, .rate = params->rate};
    const _Q15 lfoValue = updateLFO(&sLFOParams, &lfoState);

    // Split up the total modulation amount (given by LFO value) into common and differential modulation amount
    const _Q15 diffModAmount = mul_Q15_Q16(lfoValue, params->spread); // Total Mod Amount * Spread
    const _Q15 commonModAmount = mul_Q15_Q16(lfoValue, ~params->spread); // Total Mod Amount * (1-Spread)        

    // Convert chorus depth from 8-bit range to samples.
    const uint16_t depth = params->depth * DEPTH_FACTOR;
    
    // Process left stereo channel
    const uint16_t sampleReadPosL = sampleWritePos - depth - mul_Q15_Q16(mul_Q15_Q16(commonModAmount + diffModAmount, params->modDepth), depth);
    addDelayLineOutput(
            ringBufferL,
            sampleReadPosL,
            params->mix,
            dataL);

    // Process right stereo channel
    const uint16_t sampleReadPosR = sampleWritePos - depth - mul_Q15_Q16(mul_Q15_Q16(commonModAmount - diffModAmount, params->modDepth), depth);
    addDelayLineOutput(
            ringBufferR,
            sampleReadPosR,
            params->mix,
            dataR);
}
