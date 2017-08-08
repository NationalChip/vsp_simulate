/*
 * Copyright (C) 1991-2017 Nationalchip Co., Ltd
 *
 * algorithm_logfbank.h: calc logfabnk
 *
 */

#ifndef __ALGORITHM_LOGFBANK_H__
#define __ALGORITHM_LOGFBANK_H__

extern int LogFbank(int *sample, float *logfbank_out, int sample_rate, int frame_size, int fft_size, int fbank_size);

#endif

