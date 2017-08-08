/* Voice Signal Preprocess
 * Copyright (C) 1991-2017 Nationalchip Co., Ltd
 *
 * vsp_context.h: Context definition
 *
 */

#ifndef __VSP_CONTEXT_H__
#define __VSP_CONTEXT_H__

//=================================================================================================

// Audio Process Context shared with MCU, DSP and ARM
typedef struct {
    unsigned int        version;
    unsigned int        mic_num;                // 1 - 8
    unsigned int        ref_num;                // 0 - 2
    unsigned int        frame_num_per_context;  // the frame num in the context
    unsigned int        frame_num_per_channel;  // the total frame num
    unsigned int        frame_length;
    unsigned int        sample_rate;
    unsigned int        logfbanks_dim;
    unsigned int        logfbanks_group_num;
    unsigned int        far_field_pattern_num;
    unsigned int        out_buffer_size;
    short              *mic_buffer;             // should be changed to Device Address
    unsigned int        mic_buffer_size;
    short              *ref_buffer;             // should be changed to Device Address
    unsigned int        ref_buffer_size;
} VSP_CONTEXT_HEADER;

typedef struct {
    VSP_CONTEXT_HEADER *ctx_header;             // should be changed to Device Address
    unsigned            mic_mask:16;
    unsigned            ref_mask:16;
    unsigned int        frame_index;            // the first frame index
    unsigned int        ctx_index;
    unsigned int        vad:8;
    unsigned int        kws:8;
    unsigned int        gain_current:8;
    unsigned int        gain_setting:8;
    int                 direction;
    unsigned int       *farfield_pattern;
    float              *logfbanks;              // the log_fbank_buffer
    short              *out_buffer;
} VSP_CONTEXT;

//=================================================================================================

#endif /* __VSP_CONTEXT_H__ */
