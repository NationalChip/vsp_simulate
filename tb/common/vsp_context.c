/*
 * Copyright (C) 1991-2017 NationalChip Co., Ltd
 *
 * vsp_context.c : build context
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vsp_context.h>
#include <vsp_wave.h>

#define OPEN_PRINTF

#define SAMPLE_RATE             16000
#define FRAME_LENGTH			10
#define FRAME_NUM_PER_CONTEXT   3
#define CONTEXT_NUM_PER_CHANNEL 5

VSP_CONTEXT vsp_context;


static int _VspPrintContexInfo(VSP_CONTEXT *context)
{
    VSP_CONTEXT_HEADER *ctx_header  = context->ctx_header;           // should be changed to Device Address
    int frame_ms                    = ctx_header->frame_length;
    int sample_rate                 = ctx_header->sample_rate;
    int frame_num_per_context       = ctx_header->frame_num_per_context;
    int frame_num_per_channel       = ctx_header->frame_num_per_channel;
    int frame_length                = sizeof(short) * ctx_header->frame_length * ctx_header->sample_rate / 1000;
    int frame_length_per_channel    = frame_length * frame_num_per_channel;
    int current_frame_index         = context->frame_index % frame_num_per_channel;
    int frame_index                 = context->frame_index;

    printf("======================== VSP_CONTEXT ========================\n");
    printf("\tframe_ms:                 %d\n", frame_ms);
    printf("\tsample_rate:              %d\n", sample_rate);
    printf("\tmic_mask:                 %d\n", context->mic_mask);
    printf("\tref_mask:                 %d\n", context->ref_mask);
    printf("\tframe_index:              %d\n", frame_index);
    printf("\tlogfbanks_dim:            %d\n", ctx_header->logfbanks_dim);
    printf("\tlogfbanks_group_num:      %d\n", ctx_header->logfbanks_group_num);
    printf("\tmic_num:                  %d\n", ctx_header->mic_num);
    printf("\tref_num:                  %d\n", ctx_header->ref_num);
    printf("\tout_buffer_size:          %d\n", ctx_header->out_buffer_size);
    printf("\tfar_field_pattern_num:    %d\n", ctx_header->far_field_pattern_num);
    printf("\tframe_length:             %d\n", frame_length);
    printf("\tframe_num_per_context:    %d\n", frame_num_per_context);
    printf("\tframe_num_per_channel:    %d\n", frame_num_per_channel);
    printf("\tframe_length_per_channel: %d\n", frame_length_per_channel);
    printf("\tcurrent_frame_index:      %d\n", current_frame_index);
    printf("\tframe_index:              %d\n", frame_index);
    printf("\tlogfbanks_addr:           %d\n", (unsigned)context->logfbanks);
    printf("\tfarfield_pattern:         %d\n", (unsigned)context->farfield_pattern);
    printf("\tout_buffer:               %d\n", (unsigned)context->out_buffer);
    printf("======================== VSP_CONTEXT ========================\n");

    return 0;
}

static int _VspPrintWaveHeaderInfo(WAVE_HEADER hdr)
{
    char buf[5];
    printf("Header Info:\n");
    strncpy(buf, hdr.chunkID, 4);
    buf[4] = '\0';
    printf("\tChunk ID:         %s\n",buf);
    printf("\tChunk Size:       %d\n", hdr.chunkSize);
    strncpy(buf, hdr.format, 4);
    buf[4] = '\0';
    printf("\tFormat:           %s\n", buf);
    strncpy(buf, hdr.subchunkID, 4);
    buf[4] = '\0';
    printf("\tSub-chunk ID:     %s\n", buf);
    printf("\tSub-chunk Size:   %d\n", hdr.subchunkSize);
    printf("\tAudio Format:     %d\n", hdr.audioFormat);
    printf("\tChannel Count:    %d\n", hdr.numChannels);
    printf("\tSample Rate:      %d\n", hdr.sampleRate);
    printf("\tBytes per Second: %d\n", hdr.bytesPerSecond);
    printf("\tBlock alignment:  %d\n", hdr.blockAlign);
    printf("\tBit depth:        %d\n", hdr.bitDepth);
    strncpy(buf,hdr.dataID, 4);
    buf[4] = '\0';
    printf("\tData ID:          %s\n", buf);
    printf("\tData Size:        %d\n", hdr.dataSize);

    return 0;
}

int VspWriteWaveHeader(FILE *fp, int data_size)
{
    WAVE_HEADER header;
    strncpy(header.chunkID, "RIFF", 4);
    header.chunkSize = data_size * sizeof(short) + 36;
    strncpy(header.format, "WAVE", 4);
    strncpy(header.subchunkID, "fmt ", 4);
    header.subchunkSize = 16;
    header.audioFormat = 1;
    header.numChannels = 1;
    header.sampleRate = 16000;
    header.bytesPerSecond = 32000;
    header.blockAlign = 2;
    header.bitDepth = 16;
    strncpy(header.dataID, "data", 4);
    header.dataSize = data_size * sizeof(short);

    fseek(fp, 0, SEEK_SET);
    fwrite(&header, sizeof(WAVE_HEADER), 1, fp);
    return 0;
}

int VspWriteWaveData(FILE *fp, int *sample_size)
{
    VSP_CONTEXT_HEADER *ctx_header = vsp_context.ctx_header;
    int frame_size = ctx_header->frame_num_per_context * ctx_header->frame_length * SAMPLE_RATE / 1000;
    *sample_size += frame_size;
    fwrite(vsp_context.out_buffer, frame_size * sizeof(short), 1, fp);
    return 0;
}

int VspReadWaveHeader(FILE *fp)
{
    WAVE_HEADER header;

    fread(&header, sizeof(WAVE_HEADER), 1, fp);
#ifdef OPEN_PRINTF
    _VspPrintWaveHeaderInfo(header);
#endif
    if ( strncmp (header.chunkID, "RIFF", 4)
            || strncmp (header.format, "WAVE", 4)
            || strncmp (header.subchunkID , "fmt", 3)
            || strncmp (header.dataID, "data", 4)
            || header.audioFormat != 1
            || header.bitDepth != 16) {
        printf("[error] don't support file\n");
        return -1;
    }

    return 0;
}

int VspReadWaveData(FILE *fp, WAVE_DATA  *wave_data)
{
    WAVE_HEADER header;
    fseek(fp, 0, SEEK_SET);
    fread(&header, sizeof(WAVE_HEADER), 1, fp);
    wave_data->sample_size = header.dataSize/sizeof(short);
    wave_data->sample = (short *)malloc(header.dataSize);
    if (NULL == wave_data->sample) {
        printf ("[error] malloc failed\n");
    }
    fread(wave_data->sample, sizeof(short), wave_data->sample_size, fp);
    return 0;
}

int VspContextInit(void)
{
    VSP_CONTEXT_HEADER *ctx_header = (VSP_CONTEXT_HEADER *)malloc(sizeof(VSP_CONTEXT_HEADER));
    memset(ctx_header, 0, sizeof(VSP_CONTEXT_HEADER));
    ctx_header->version = 20170731;
    ctx_header->mic_num = 6;
    ctx_header->ref_num = 2;
    ctx_header->frame_num_per_context = FRAME_NUM_PER_CONTEXT;
    ctx_header->frame_num_per_channel = CONTEXT_NUM_PER_CHANNEL * FRAME_NUM_PER_CONTEXT;
    ctx_header->frame_length = FRAME_LENGTH;
    ctx_header->sample_rate = SAMPLE_RATE;
    ctx_header->logfbanks_dim = 40;
    ctx_header->logfbanks_group_num = 8;
    ctx_header->far_field_pattern_num = 360;
    int frame_size = ctx_header->frame_num_per_channel * ctx_header->frame_length * SAMPLE_RATE / 1000;
    ctx_header->out_buffer_size = frame_size * sizeof(short);
    ctx_header->mic_buffer_size = ctx_header->mic_num * frame_size * sizeof(short);
    ctx_header->mic_buffer = (short *)malloc(ctx_header->mic_buffer_size);
    ctx_header->ref_buffer_size = ctx_header->ref_num * frame_size * sizeof(short);
    ctx_header->ref_buffer = (short *)malloc(ctx_header->ref_buffer_size);

    vsp_context.ctx_header = ctx_header;
    vsp_context.mic_mask = 0xff;
    vsp_context.ref_mask = 0xff;
    vsp_context.frame_index = -3;
    vsp_context.ctx_index = -1;
    vsp_context.vad = 0;
    vsp_context.kws = 0;
    vsp_context.gain_current = 0;
    vsp_context.gain_setting = 0;
    vsp_context.farfield_pattern = (unsigned int *)malloc(ctx_header->far_field_pattern_num * sizeof(unsigned int));
    vsp_context.logfbanks = (float *)malloc(ctx_header->logfbanks_group_num * ctx_header->logfbanks_dim);
    vsp_context.out_buffer = (short *)malloc(ctx_header->out_buffer_size);

#ifdef OPEN_PRINTF
    _VspPrintContexInfo(&vsp_context);
#endif
    return 0;
}

VSP_CONTEXT* VspGetContext(WAVE_DATA mic_data[], int mic_num, WAVE_DATA ref_data[], int ref_num)
{
    VSP_CONTEXT_HEADER *ctx_header = vsp_context.ctx_header;
    vsp_context.ctx_index++;
    vsp_context.frame_index += ctx_header->frame_num_per_context;
    
    int frame_size = ctx_header->frame_num_per_context * ctx_header->frame_length * SAMPLE_RATE / 1000;
    int channel_size = ctx_header->frame_num_per_channel * ctx_header->frame_length * SAMPLE_RATE / 1000;
    int offset = vsp_context.ctx_index* frame_size;
    int current_ctx_index = vsp_context.ctx_index % (ctx_header->frame_num_per_channel / ctx_header->frame_num_per_context);

    for (int i=0; i<mic_num; i++) {
        if (offset + frame_size > mic_data[i].sample_size) {
            return NULL;
        }
        short *src = mic_data[i].sample + offset;
        short *dest = ctx_header->mic_buffer + current_ctx_index * frame_size + i * channel_size;
        memcpy (dest, src, frame_size * sizeof(short));
    }

    for (int i=0; i<ref_num; i++) {
        if (offset + frame_size > ref_data[i].sample_size) {
            return NULL;
        }
        short *src = ref_data[i].sample + offset;
        short *dest = ctx_header->ref_buffer+ current_ctx_index * frame_size + i * channel_size;
        memcpy (dest, src, frame_size * sizeof(short));
    }

    return &vsp_context;
}


