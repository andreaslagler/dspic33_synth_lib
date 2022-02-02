#ifndef FORMANT_FILTER_TYPES_H
#define	FORMANT_FILTER_TYPES_H

#include "svf_2pole_types.h"

#define NOF_FORMANT_FILTER_STAGES 4

typedef struct
{
    SVF2PoleState states[NOF_FORMANT_FILTER_STAGES];
} FormantFilterState;


#endif

