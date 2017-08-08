/* Algorithm implement
 * Copyright (C) 1991-2017 Nationalchip Co., Ltd
 *
 * vsp_algorithm_implement.c
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <xtensa/config/core.h>

#include "vsp_context.h"
#include "algorithm/algorithm_common.h"
#include "algorithm/algorithm_fft.h"
#include "algorithm/algorithm_logfbank.h"

#define FFT_SIZE            512
#define FRAME_SIZE          480 // 30ms
#define WIN_SIZE            400 // 25ms
#define SHIFT_SIZE          160 // 10ms
#define GROUP_NUM           8   // frame nums
#define LOGFBANKS_DIM       40
#define TMP_BUFFER_SIZE     160*6 // >=160*6 don't modify the value, used for agc and other

//#define OPEN_PRINTF
//#define OPEN_DEBUG_PRINTF

static int preemph_buffer_tmp[FRAME_SIZE];                  // 480 samples
static int preemph_buffer[FRAME_SIZE];                      // 480 samples
static int fft_buffer[FFT_SIZE/2 + 1];                      //
static int real_buffer[FFT_SIZE/2 + 1];                     //
static int image_buffer[FFT_SIZE/2 + 1];                    //
static short tmp_buffer[TMP_BUFFER_SIZE];                   // used for logfbank
static float log_fbank_buffer[GROUP_NUM * LOGFBANKS_DIM];   //

const float fbank_means[] = {
	4.71739399, 5.65092761, 6.76021255, 7.77460884, 8.27439655,
    8.18172773, 8.37250101, 8.55928316, 8.85503389, 8.73348498,
    8.57087729, 8.69329829, 8.64948065, 8.6654904, 8.8717032,
    9.04465497, 9.05919717, 9.09013292, 9.26431323, 9.34897128,
    9.25719405, 9.19238699, 9.29047772, 9.44815699, 9.63362215,
    9.75744786, 9.81185688, 9.78490666, 9.84767836, 9.98264263,
    9.99049155, 10.09759631, 10.1189895, 10.1271409, 10.20729083,
    10.22023744, 10.28798183, 10.35491619, 10.26507241, 9.99242738};
const float fbank_stds[] = {
    3.71597166, 3.5104997, 3.58941071, 3.81598394, 3.93445797, 3.75909449,
    3.83470532, 3.96048596, 3.90193349, 3.74952077, 3.58299029, 3.44647646,
    3.30860122, 3.20787963, 3.12544712, 3.06684181, 3.02312342, 3.01678436,
    3.01764255, 2.97867196, 2.90333096, 2.84332356, 2.81676655, 2.83071253,
    2.87038226, 2.88538955, 2.85437223, 2.82267158, 2.80709582, 2.80129757,
    2.79815081, 2.78466517, 2.7462945, 2.70080385, 2.68012301, 2.67179063,
    2.68659198, 2.73527099, 2.77980284, 2.87816292};


#define QN 15
int _VspPrintfFFT(int *real_buffer, int *image_buffer, int shift_num)
{
    if (-1 == shift_num) {
        return -1;
    }

    for (int j = 0; j < sizeof(fft_buffer) / sizeof(int); j++) {
        int cpl_model_qn = QN - shift_num + QN -shift_num;
        long long cpl_model = (long long)real_buffer[j] * (long long)real_buffer[j] + (long long)image_buffer[j] * (long long)image_buffer[j];//Q((15-shift_num)*2)
        int bit_length = BitLength(cpl_model);
        int qn = (bit_length - (cpl_model_qn - 15 +9))  > 32 ? (bit_length - (cpl_model_qn - 15 +9)) - 32 : 0;
        fft_buffer[j] = cpl_model >> (9 + qn + cpl_model_qn - 15);
        if(j%10 == 0)printf("\n");
        printf("%5.5f ", UnsignQnToFloat(fft_buffer[j], 15 - qn));
    }

    return 0;
}

int VspDoFFT(VSP_CONTEXT *context)
{
    VSP_CONTEXT_HEADER *ctx_header  = context->ctx_header;           // should be changed to Device Address
    int frame_length                = ctx_header->frame_length * ctx_header->sample_rate / 1000;
    int shift_num                   = 0;

    if (-1 == PreEmph(ctx_header->mic_buffer, preemph_buffer_tmp, ctx_header->frame_num_per_context * frame_length)) {
        return -1;
    }

#ifdef OPEN_PRINTF
    printf("\nfft_data:");
#endif

    if (context->frame_index == 0) {
        memcpy(preemph_buffer, preemph_buffer_tmp, sizeof(int) * FRAME_SIZE);
        shift_num = FFT(preemph_buffer, real_buffer, image_buffer, WIN_SIZE, FFT_SIZE);
#ifdef OPEN_DEBUG_PRINTF
        _VspPrintfFFT(real_buffer, image_buffer, shift_num);
#endif
    }
    else {
        for (int i=0; i<ctx_header->frame_num_per_context; i++) {
            memcpy(&preemph_buffer[FRAME_SIZE - SHIFT_SIZE], &preemph_buffer_tmp[i * SHIFT_SIZE], sizeof(int) * SHIFT_SIZE);
            shift_num = FFT(preemph_buffer, real_buffer, image_buffer, WIN_SIZE, FFT_SIZE);
#ifdef OPEN_DEBUG_PRINTF
            _VspPrintfFFT(real_buffer, image_buffer, shift_num);
#endif
            memcpy(preemph_buffer, &preemph_buffer[SHIFT_SIZE], sizeof(int) * (FRAME_SIZE - SHIFT_SIZE));
        }
    }

    memcpy(preemph_buffer, &preemph_buffer[SHIFT_SIZE], sizeof(int) * (FRAME_SIZE - SHIFT_SIZE));
    return 0;
}

int VspDoLogfbank(VSP_CONTEXT *context)
{
    VSP_CONTEXT_HEADER *ctx_header  = context->ctx_header;           // should be changed to Device Address
    int frame_num_per_context       = ctx_header->frame_num_per_context;
    int frame_num_per_channel       = ctx_header->frame_num_per_channel;
    int frame_length                = ctx_header->frame_length * ctx_header->sample_rate / 1000;
    int frame_length_per_channel    = frame_length * frame_num_per_channel;
    int current_frame_index         = context->frame_index % frame_num_per_channel;
    int valid_mic_index             = 2;
    int fb_order                    = ctx_header->logfbanks_dim;
    int fbank_shift_frame_num       = ctx_header->logfbanks_group_num - frame_num_per_context;
    short *valid_current_mic_buffer = ctx_header->mic_buffer + valid_mic_index * frame_length_per_channel + current_frame_index * frame_length;
    float fbank[fb_order];
    
    if (-1 == PreEmph(valid_current_mic_buffer, preemph_buffer_tmp, ctx_header->frame_num_per_context * frame_length)) {
        return -1;
    }

    if(context->frame_index == 0) {
        memcpy(preemph_buffer, &preemph_buffer_tmp[SHIFT_SIZE], sizeof(int) * (FRAME_SIZE - SHIFT_SIZE));
        return -1;
    }

    // fbank shift
    if (ctx_header->logfbanks_group_num > frame_num_per_context) {
        memcpy(tmp_buffer, &log_fbank_buffer[frame_num_per_context * ctx_header->logfbanks_dim]
                , fbank_shift_frame_num * ctx_header->logfbanks_dim * sizeof(float));
        memcpy(log_fbank_buffer, tmp_buffer
                ,  fbank_shift_frame_num * ctx_header->logfbanks_dim * sizeof(float));
        memset(&log_fbank_buffer[fbank_shift_frame_num* ctx_header->logfbanks_dim], 0
                , frame_num_per_context * ctx_header->logfbanks_dim * sizeof(float));
    }

    for (int j = 0; j < ctx_header->frame_num_per_context; j++) {
        memcpy(&preemph_buffer[FRAME_SIZE - SHIFT_SIZE], &preemph_buffer_tmp[j * SHIFT_SIZE], sizeof(int) * SHIFT_SIZE);
        LogFbank(preemph_buffer, fbank, ctx_header->sample_rate, WIN_SIZE, FFT_SIZE, ctx_header->logfbanks_dim);
        for (int i=0; i<fb_order; i++) {
            int index = (fbank_shift_frame_num + j) * ctx_header->logfbanks_dim + i;
            log_fbank_buffer[index] = (fbank[i] - fbank_means[i]) / fbank_stds[i]; // 2^25
        }
#ifdef OPEN_PRINTF
        for(int i = 0; i<40; i++) {
            if(i%10==0) printf("\n");
            printf("%16.5f", fbank[i]);
        }
#endif
        memcpy(preemph_buffer, &preemph_buffer[SHIFT_SIZE], sizeof(int) * (FRAME_SIZE - SHIFT_SIZE));
    }

    memcpy(context->logfbanks, log_fbank_buffer, ctx_header->logfbanks_group_num * ctx_header->logfbanks_dim * sizeof(float));
    memcpy(preemph_buffer, &preemph_buffer[SHIFT_SIZE], sizeof(int) * (FRAME_SIZE - SHIFT_SIZE));

#ifdef OPEN_PRINTF
    for(int i = 0; i < ctx_header->logfbanks_group_num * ctx_header->logfbanks_dim; i++) {
        if(i%10==0) printf("\n");
        printf("%16.5f", context->logfbanks[i]);
    }
#endif

    return 0;
}
