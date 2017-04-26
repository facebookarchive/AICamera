// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <sys/types.h>

#import <TargetConditionals.h>
#import <Metal/Metal.h>

typedef struct FBMetalCNNConstantValues {
  ushort output_width;
  ushort output_height;
  ushort input_width;
  ushort input_height;
  ushort stride_x;
  ushort stride_y;
  ushort filterSize;
  ushort channels;
  ushort kernel_stride;
  ushort kernels;
} FBMetalCNNConstantValues;

#if __IPHONE_OS_VERSION_MIN_REQUIRED > 1000
extern "C" MTLFunctionConstantValues* makeMetalConstantValues(const FBMetalCNNConstantValues* compiler_constants);
#endif
