/*
 * Copyright (C) 1991-2017 NationalChip Co., Ltd
 *
 * algorithm_logfbank.c: calc logfbank
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NatureDSP_types.h"
#include "NatureDSP_Signal.h"
#include "algorithm/algorithm_common.h"

//#define OPEN_PRINTF
//#define OPEN_DEBUG_PRINTF

#define QN    15

int _FFT(int *sample, float *fft_out, int frame_size, int fft_size)
{
    int bexp = 0;
    int shift_num = 0;
    static int *x = NULL;
    static int *fft_cpl_out = NULL;

    if (!x || !fft_cpl_out) {
        x = malloc(sizeof(int) * fft_size);
        fft_cpl_out = malloc(sizeof(int) * (fft_size + 2));
        if (!x || !fft_cpl_out) {
            printf("[error], malloc x or fft_cpl_out failed, %s, %d\n", __FILE__, __LINE__);
            return -1;
        }
    }

    fft_handle_t h = h_fft_real_x32_512;
    memcpy(x, sample, frame_size*sizeof(int));
    for(int i=frame_size; i<fft_size; i++) {
        x[i] = 0;
    }
    shift_num = fft_real32x32(fft_cpl_out, x, bexp, h);

#ifdef OPEN_DEBUG_PRINTF
    printf("fft_data:\n");
#endif
    for(int i=0; i<fft_size/2+1; i++) {
        int real = fft_cpl_out[2*i];
        int image = fft_cpl_out[2*i + 1];
        int cpl_model_qn = QN - shift_num + QN -shift_num;
        long long cpl_model = (long long)real * (long long)real + (long long)image * (long long)image;//Q((15-shift_num)*2)
        int qn = 0;
        // int bit_length = 0;
        // bit_length = BitLength(cpl_model);
        // qn = (bit_length - (cpl_model_qn - QN + 9))  > 32 ? (bit_length - (cpl_model_qn - QN + 9)) - 32 : 0;
        // optimization cycles
        if ( cpl_model < 281474976710656) { // 1 << 48
            qn = 0;
        }
        else if (cpl_model < 72057594037927936) { // 1 << 56
            qn = 8;
        }
        else {
            qn = QN;
        }
        fft_out[i] = (float)(cpl_model >> (9 + qn + cpl_model_qn - QN)) / (1 << (QN - qn));
#ifdef OPEN_DEBUG_PRINTF
        if(i%10 == 0)printf("\n");
        printf("%5.5f ", fft_out[i]);
#endif
    }
    return 0;
}

// in:q0 out:q25
long long _FreqToMel(int freq)
{
    // algorithm: 2595 * log10(freq / 700.0 + 1.0)
    int log_data = scl_log10_32x32((freq<<QN) / 700 + (1<<QN)); // q25
    long long mel = (long long)2595 * log_data; // q25
    return mel;
}

// in:q25 out:q15
int  _MelToFreq(long long mel)
{
    //algorithm: 700 * (pow(10, (mel / 2595.0)) - 1);
    unsigned int x = mel / 2595;
    int y = scl_antilog10_32x32(x); // q15
    int freq = 700 * (y - (1<<QN));
    return freq;
}

int _MelFilter(float *fft_data, float *fbank_out, int sample_rate, int fft_size, int fbank_size)
{
    static unsigned int *mel_point = NULL;
    static float *mel_weight = NULL;

    if (!mel_point || !mel_weight) {
        mel_point = malloc (sizeof(unsigned int) * (fbank_size + 2));
        mel_weight = malloc (sizeof(float) * ((fft_size>>1) +1));
        if (!mel_point || !mel_weight) {
            printf("[error], malloc mel_point or mel_weight failed, %s, %d\n", __FILE__, __LINE__);
        }

        long long max_mel = _FreqToMel(sample_rate>>1); // in:q0 out:q25
#ifdef OPEN_DEBUG_PRINTF
        printf("\nmax_mel:%16.5f\n", (float)max_mel/(1<<25));
        printf("mel_point:");
#endif
        for (int i = 0; i < fbank_size + 2; i++) {
            long long mel = max_mel * i / (fbank_size + 1); // q25
            mel_point[i] = (unsigned int)(((long long)(fft_size + 1) * _MelToFreq(mel)) / sample_rate); // q15
#ifdef OPEN_DEBUG_PRINTF
            printf("%16.5f", (float)mel_point[i]/(1<<QN));
#endif
        }

#ifdef OPEN_DEBUG_PRINTF
        printf("\nmel_weight:\n");
#endif
        mel_weight[0] = 0;
        for (int i = 0; i < fbank_size + 1; i++) {
            unsigned int point1 = mel_point[i]>>QN;
            unsigned int point2 = mel_point[i + 1]>>QN;
            for (int j = point1 + 1; j <= point2; j++ ) {
                mel_weight[j] = (float)(j - point1) / (point2 - point1);
#ifdef OPEN_DEBUG_PRINTF
                printf("%16.5f[%d]", mel_weight[j], j);
#endif
            }
        }
    }

    for (int k = 0; k <= fbank_size; k++) {
        int point1 = (int)mel_point[k]>>QN;
        int point2 = (int)mel_point[k+1]>>QN;

        for (int j = (point1 + 1); j <= point2; j++) {
            if (k < fbank_size)
                fbank_out[k] += mel_weight[j] * fft_data[j];//* (1<<QN));
            if (k > 0)
                fbank_out[k-1] += (1 - mel_weight[j]) * fft_data[j];// * (1<<QN));
#ifdef OPEN_DEBUG_PRINTF
            printf("%16.5f%16.5f%16.5f%16.5f[%d:%d]\n", fbank_out[k], fbank_out[k-1], mel_weight[j], fft_data[j], j, k);
#endif
        }
        if (k > 0)
            fbank_out[k-1] = (fbank_out[k-1] < 0.01)?0.01:fbank_out[k-1];
#ifdef OPEN_DEBUG_PRINTF
        if(k > 0) printf("#%16.5f\n", fbank_out[k-1]);
#endif
    }
    return 0;
}

int _Log(float *fbank, float *logfbank_out, int fbank_size)
{
#ifdef OPEN_PRINTF
    printf("\nlogfbank:");
#endif
    for (int i = 0; i < fbank_size; i++) {
        int adj = 0;
        long long l = fbank[i]*(1<<QN);
        int overflow = l>>31;
        float div = 0.0f;
        if (overflow < 3) {
            adj = 1;
            div = 2.718281828459045;
        }
        else if (overflow < 8) {
            adj = 2;
            div = 7.38905609893065;
        }
        else if (overflow < 21) {
            adj = 3;
            div = 20.085536923187668;
        }
        else if (overflow < 55) {
            adj = 4;
            div = 54.598150033144236;
        }
        else if (overflow < 149) {
            adj = 5;
            div  = 148.4131591025766;
        }
        else if (overflow < 404) {
            adj = 6;
            div = 403.4287934927351;
        }
        else if (overflow < 1097) {
            adj = 7;
            div = 1096.6331584284585;
        }
        else if (overflow < 2981) {
            adj = 8;
            div = 2980.9579870417283;
        }
        int x = l/div;
        int log_data = scl_logn_32x32(x);
        logfbank_out[i] = (float)log_data/(1<<25) + adj;
#ifdef OPEN_PRINTF
        if (i%10 == 0) printf("\n");
        printf("%16.5f", logfbank_out[i]);
#endif
    }
#ifdef OPEN_PRINTF
        printf("\n");
#endif
    return 0;
}

int LogFbank(int *sample, float *logfbank_out, int sample_rate, int frame_size, int fft_size, int fbank_size)
{
    static float *fft_data = NULL;
    static float *fbank_data = NULL;

    if (!fft_data || !fbank_data) {
        fft_data = malloc(sizeof(float) * ((fft_size>>1) +1));
        fbank_data = malloc(sizeof(float) * (fbank_size));
        if (!fft_data || !fbank_data) {
            printf("[error], malloc fbank_data or fft_data failed, %s, %d\n", __FILE__, __LINE__);
            return -1;
        }
    }
    
    memset(fbank_data, 0 , sizeof(float)*fbank_size);
    _FFT(sample, fft_data, frame_size, fft_size);
    _MelFilter(fft_data, fbank_data, sample_rate, fft_size, fbank_size);
    _Log(fbank_data, logfbank_out, fbank_size);
   
    return 0;
}


