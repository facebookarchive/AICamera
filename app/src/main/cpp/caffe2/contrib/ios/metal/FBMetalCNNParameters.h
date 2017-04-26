// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

// This header file is shared between C code and Metal code

#define MAX_KERNELS_PER_CONVOLUTION 32

typedef struct {
#ifdef __METAL_VERSION__
  half bias[MAX_KERNELS_PER_CONVOLUTION];
#else
  float16_t bias[MAX_KERNELS_PER_CONVOLUTION];
#endif
} FBMetalCNNParameters;
