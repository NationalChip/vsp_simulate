/*
 * Copyright (C) 1991-2017 NationalChip Co., Ltd
 *
 * main.c:
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xtensa/config/core.h>
#include <vsp_context.h>
#include <vsp_wave.h>

#include "vsp_process.h"

//#define CALC_PROFILE
#define MIC_NUM  4
#define REF_NUM  2

static char path_buffer[256];
int main(int argc, char *argv[])
{
    FILE *fp_input_mic[MIC_NUM], *fp_input_ref[REF_NUM],*fp_out_wav;
    WAVE_DATA mic_data[MIC_NUM], ref_data[REF_NUM];

    if (argc < 2) {
        printf("dsp_tb <input dir>\n");
        return -1;
    }

    char file_name[20];
    // read mic wav
    for (int i=0; i<MIC_NUM; i++) {
        strncpy(path_buffer, argv[1], sizeof(path_buffer));
        sprintf(file_name, "/mic%d.wav", i);
        strcat(path_buffer, file_name);
        if ((fp_input_mic[i] = fopen(path_buffer,"r")) == NULL) {
            printf("cannot read file [%s]\n", path_buffer);
            return -1;
        }
        if (0 != VspReadWaveHeader(fp_input_mic[i])) {
            printf("[error] %s is not wave\n", path_buffer);
            return -1;
        }
    }

    // read ref wav
    for (int i=0; i<REF_NUM; i++) {
        strncpy(path_buffer, argv[1], sizeof(path_buffer));
        sprintf(file_name, "/ref%d.wav", i);
        strcat(path_buffer, file_name);
        if ((fp_input_ref[i] = fopen(path_buffer,"r")) == NULL) {
            printf("cannot read file [%s]\n", path_buffer);
            return -1;
        }
        if (0 != VspReadWaveHeader(fp_input_ref[i])) {
            printf("[error] %s is not wave\n", path_buffer);
            return -1;
        }
    }

    // 
    for (int i=0; i<MIC_NUM; i++) {
        VspReadWaveData(fp_input_mic[i], &mic_data[i]);
    }
    for (int i=0; i<REF_NUM; i++) {
        VspReadWaveData(fp_input_ref[i], &ref_data[i]);
    }

    strncpy(path_buffer, argv[1], sizeof(path_buffer));
    sprintf(file_name, "/out.wav");
    strcat(path_buffer, file_name);
    if ((fp_out_wav= fopen(path_buffer,"wb")) == NULL) {
        printf("cannot read file [%s]\n", path_buffer);
        return -1;
    }
    VspWriteWaveHeader(fp_out_wav, 0);

    VspContextInit();
    VSP_CONTEXT *context;
    int sample_size = 0;
    int cnt = 0;
    while ((context = VspGetContext(mic_data, MIC_NUM, ref_data, REF_NUM)) != NULL) {
        printf ("VSP process start :%d\n", ++cnt);
        // do aec denoise beamforming doa logfbank
        VspProcessActive(context);

        // save out_data
        VspWriteWaveData(fp_out_wav, &sample_size);
#ifdef CALC_PROFILE
        if (cnt >= 4) break;
#endif
    }
    VspWriteWaveHeader(fp_out_wav, sample_size);

    for (int i=0; i<MIC_NUM; i++) {
        fclose(fp_input_mic[i]);
    }
    for (int i=0; i<REF_NUM; i++) {
        fclose(fp_input_ref[i]);
    }
    fclose(fp_out_wav);

    return 0;
}

