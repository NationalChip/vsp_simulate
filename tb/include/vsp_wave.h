/*
 * Copyright (C) 1991-2017 Nationalchip Co., Ltd
 *
 * vsp_wave.h: Wave definition
 *
 */

#ifndef __VSP_WAVE_H__
#define __VSP_WAVE_H__

#include <stdio.h>
#include <vsp_context.h>

//=================================================================================================

// Wave header and data, used for deal wave file
typedef struct {
    char            chunkID[4];         // 1-4      "RIFF"
    int             chunkSize;          // 5-8
    char            format[4];          // 9-12     "WAVE"
    char            subchunkID[4];      // 13-16    "fmt\0"
    int             subchunkSize;       // 17-20
    unsigned short  audioFormat;        // 21-22    PCM = 1
    unsigned short  numChannels;        // 23-24
    int             sampleRate;         // 25-28
    int             bytesPerSecond;     // 29-32
    unsigned short  blockAlign;         // 33-34
    unsigned short  bitDepth;           // 35-36    16bit support only
    char            dataID[4];          // 37-40    "data"
    int             dataSize;           // 41-44
} WAVE_HEADER;

typedef struct {
    short           *sample;
    unsigned int     sample_size;
} WAVE_DATA;

extern int VspWriteWaveHeader(FILE *fp, int data_size);
extern int VspWriteWaveData(FILE *fp, int *sample_size);
extern int VspReadWaveHeader(FILE *fp);
extern int VspReadWaveData(FILE *fp, WAVE_DATA  *wave_data);
extern int VspContextInit(void);
extern VSP_CONTEXT* VspGetContext(WAVE_DATA mic_data[], int mic_num, WAVE_DATA ref_data[], int ref_num);

#endif

