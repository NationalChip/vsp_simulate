/*
 * Copyright (C) 1991-2017 Nationalchip Co., Ltd
 *
 * algorithm_common.c: generic algorithm implementation
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "NatureDSP_types.h"
#include "NatureDSP_Signal.h"

#define FRAME_SIZE  480 // 30ms

//#define OPEN_PRINTF

int FloatToSignQn(float f, int qn)
{
    return (int)(f * (1<<qn));
}

float UnsignQnToFloat(unsigned int i, int qn)
{
    return (float)i / (1<<qn);
}

float SignQnToFloat(int i, int qn)
{
    return (float)i / (1<<qn);
}

int BitLength(long long n)
{
    // pow of 2, 2^0 - 2 ^64
    long long powof2[64] =
    {
        1ll<<0 ,	1ll<<1 ,	1ll<<2 ,	1ll<<3 ,	1ll<<4 ,	1ll<<5 ,	1ll<<6 ,	1ll<<7 ,
        1ll<<8 ,	1ll<<9 ,	1ll<<10,	1ll<<11,	1ll<<12,	1ll<<13,	1ll<<14,	1ll<<15,
        1ll<<16,	1ll<<17,	1ll<<18,	1ll<<19,	1ll<<20,	1ll<<21,	1ll<<22,	1ll<<23,
        1ll<<24,	1ll<<25,	1ll<<26,	1ll<<27,	1ll<<28,	1ll<<29,	1ll<<30,	1ll<<31,
        1ll<<32,	1ll<<33,	1ll<<34,	1ll<<35,	1ll<<36,	1ll<<37,	1ll<<38,	1ll<<39,
        1ll<<40,	1ll<<41,	1ll<<42,	1ll<<43,	1ll<<44,	1ll<<45,	1ll<<46,	1ll<<47,
        1ll<<48,	1ll<<49,	1ll<<50,	1ll<<51,	1ll<<52,	1ll<<53,	1ll<<54,	1ll<<55,
        1ll<<56,	1ll<<57,	1ll<<58,	1ll<<59,	1ll<<60,	1ll<<61,	1ll<<62,	1ll<<63,
    } ;
    int left = 0;
    int right = 63;

    while (left <= right) {
        int mid = (left + right) / 2 ;
        if (powof2[mid] <= n)
        {
            if (powof2[mid + 1] > n)
                return mid + 1; // got it!
            else // powof2[mid] < n, search right part
                left = mid + 1 ;
        }
        else // powof2[mid] > n, search left part
            right = mid - 1 ;
    }
    // not found
    return -1 ;
}

#define QN  15
int PreEmph(short *sample, int *preemph_sample, int sample_size)
{
    static int last_sample = 0;

    if (!sample || !preemph_sample) {
        printf("[error] sample or preEmph_sample is null %s %d\n", __FILE__, __LINE__);
    }

    if (sample_size > FRAME_SIZE) {
        printf("[error] sample_size is larger than FRAME_SIZE!! %s %d\n", __FILE__, __LINE__);
        return -1;
    }

    for (int i=0; i<sample_size; i++) {
        preemph_sample[i] = FloatToSignQn((float)sample[i] - (float)last_sample * 0.97, QN); // 0.96875 ≈ 0.97
        //preemph_sample[i] = (sample[i]<<15) - (last_sample<<15) + (last_sample<<10); // 0.96875 ≈ 0.97
        last_sample = sample[i];
    }

    return 0;
}


