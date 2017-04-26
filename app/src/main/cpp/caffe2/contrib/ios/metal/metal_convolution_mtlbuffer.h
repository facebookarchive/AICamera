// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#import <Metal/Metal.h>

bool metal_convolution(
    id<MTLBuffer> inputBuffer,
    int           input_channels,
    int           input_width,
    int           input_height,
    int           stride_x,
    int           stride_y,
    id<MTLBuffer> weightBuffer,
    int           kernels,
    int           kernel_channels,
    int           kernel_width,
    int           kernel_height,
    id<MTLBuffer> outputBuffer,
    int           output_number,
    int           output_width,
    int           output_height,
    const float*  bias,
    int           bias_length,
    bool          transposed);
