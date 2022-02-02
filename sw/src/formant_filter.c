#include "FormantFilter.h"
#include "SVF_2Pole.h"
#include <stdint.h>
#include "fp_lib.h"
#include "block_len_def.h"

#define NOF_VOWELS 5

static inline int16_t CalcLinearInterpolation(
const int16_t y1,
const int16_t y2,
const int16_t x)
{
        int16_t y = 0;
        __asm__ volatile(
            "\
        lac     %[Y1], #0, A                                                       ;AccA = X1[k] \n \
        msc     %[Y1] * %[X], A                                                  ;AccA -= X1[k] * Mix => AccA = X1[k] * (1-Mix) \n \
        mac     %[Y2] * %[X], A                                                  ;AccA += X2[k] * Mix => AccA = X1[k] * (1-Mix) + X2[k] * Mix \n \
        sac.r   A, #0, %[Y]                                                ;X2[k] = AccB \n \
                                                                                ;\n \
        ; 4 cycles total"
            : [Y]"+r"(y)  /*out*/
            : [Y1]"z"(y1), [Y2]"z"(y2), [X]"z"(x) /*in*/
            : /*clobbered*/
            );
            
            return y;
}

//
static inline uint16_t GetFreq(
        const uint16_t uiStage,
        const _Q16 qShape)
{
    static const uint16_t auiFreq[NOF_VOWELS][NOF_FORMANT_FILTER_STAGES] = {
{15647, 17236, 19944, 21248},
{14021, 19096, 20065, 20854},
{12321, 19498, 20656, 21248},
{14014, 15787, 20025, 20757},
{12610, 17328, 19772, 20886}
    };
    
    const uint16_t uiVowel = qShape >> 14;
    const int16_t iShape = (qShape << 2) >> 1;
    return CalcLinearInterpolation(auiFreq[uiVowel][uiStage], auiFreq[uiVowel+1][uiStage], iShape) << 1;
}

//
static inline _Q16 GetQ(
        const uint16_t uiStage,
        const _Q16 qShape)
{
    static const _Q16 aqQ[NOF_VOWELS][NOF_FORMANT_FILTER_STAGES] = {
{30798, 31248, 31694, 31684},
{29609, 31880, 31731, 31554},
{27566, 31978, 31894, 31684},
{29603, 30455, 31719, 31519},
{27992, 31288, 31639, 31565}
    };
    
    const uint16_t uiVowel = qShape >> 14;
    const int16_t iShape = (qShape << 2) >> 1;
    return CalcLinearInterpolation(aqQ[uiVowel][uiStage], aqQ[uiVowel+1][uiStage], iShape) << 1;
}

//
void CalcFormantFilterInplace(
        const unsigned int uiFreq,
        const _Q16 qShape,
        SFORMANT_FILTER_STATE * psState,
        _Q15 * pqData)
{
    uint16_t uiCFilterStage;
    for (uiCFilterStage = 0; uiCFilterStage < NOF_FORMANT_FILTER_STAGES; ++uiCFilterStage)
    {

        const unsigned int uiCutOff = uiFreq + GetFreq(
                uiCFilterStage,
                qShape);

        const _Q16 qQ = GetQ(
                uiCFilterStage,
                qShape);

        CalcLP2PoleInplace(
                uiCutOff,
                qQ,
                &psState->asStates[uiCFilterStage],
                pqData);
    }
}

