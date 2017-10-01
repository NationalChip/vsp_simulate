/*
 * Copyright (C) 1991-2017 NationalChip Co., Ltd
 *
 * algorithm_fft.cc:
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NatureDSP_types.h"
#include "NatureDSP_Signal.h"

extern "C" {
    int FFT(int *sample, int *real, int *image, int frame_size, int fft_size);
}

int FFT(int *sample, int *real, int *image, int frame_size, int fft_size)
{
    int bexp = 0;
    int shift_num = 0;
    int *x = (int *)malloc(sizeof(int) * fft_size);
//    int *fft_cpl_out = (int *)malloc(sizeof(int) * (fft_size + 2));
    int *fft_cpl_out = new int[sizeof(int) * (fft_size + 2)];//(int *)malloc(sizeof(int) * (fft_size + 2));

    if (x == NULL || fft_cpl_out == NULL) {
        return -1;
    }

    fft_handle_t h = h_fft_real_x32_512;

    memcpy(x, sample, frame_size*sizeof(int));
    for(int i=frame_size; i<fft_size; i++) {
        x[i] = 0;
    }

    shift_num = fft_real32x32(fft_cpl_out, x, bexp, h);

    for(int i=0; i<fft_size/2+1; i++) {
        real[i] = fft_cpl_out[2*i];
        image[i] = fft_cpl_out[2*i + 1];
    }

    free(x);
    free(fft_cpl_out);

    return shift_num;
}

