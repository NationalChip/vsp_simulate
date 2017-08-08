/* Voice Signal Preprocess
 * Copyright (C) 1991-2017 Nationalchip Co., Ltd
 *
 * vsp_process.c: Process a context
 *
 */

#include <stdlib.h>
#include <string.h>
#include <vsp_context.h>

#include "vsp_algorithm_implement.h"

int VspProcessActive(VSP_CONTEXT *context)
{
    int result = -1;

    //result = VspDoFFT(context);
    result = VspDoLogfbank(context);
    return result;
}

