// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

bool calculate_kernels_per_convolution(int& kernels_per_convolution);

int calculate_kernel_stride(int kernels_per_convolution);

bool metal_convolution(
    const float* in,
    int          in_channels,
    int          in_width,
    int          in_height,
    int          stride_x,
    int          stride_y,
    const float* kernel,
    int          kernel_number,
    int          kernel_channels,
    int          kernel_width,
    int          kernel_height,
    float*       out,
    int          output_number,
    int          out_width,
    int          out_height,
    const float* bias,
    int          bias_length,
    bool         transpose_kernel);
