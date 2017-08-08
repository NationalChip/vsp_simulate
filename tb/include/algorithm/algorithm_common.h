/*
 * Copyright (C) 1991-2017 Nationalchip Co., Ltd
 *
 * algorithm_common.h
 *
 */
#ifndef __ALGORITHM_COMMON__
#define __ALGORITHM_COMMON__


extern int FloatToSignQn(float f, int qn);
extern float UnsignQnToFloat(unsigned int i, int qn);
extern float SignQnToFloat(int i, int qn);
extern int BitLength(long long n);
extern int PreEmph(short *sample, int *preemph_sample, int sample_size);

#endif

