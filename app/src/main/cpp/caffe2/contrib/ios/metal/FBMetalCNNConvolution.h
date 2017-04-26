// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include "arm_neon_support.h"

#import "FBMetalCNNParameters.h"
#import "MetalImageFilter.h"
#import "FBMetalCNNConstantValues.h"

static constexpr size_t kThreadGroupSize_x = 4;
static constexpr size_t kThreadGroupSize_y = 8;

typedef float32_t data_buffer_type;

typedef float16_t weight_buffer_type;

@interface FBMetalCNNConvolution : MetalImageFilter

@property (nonatomic, strong) id<MTLBuffer> dataBuffer;

@property (nonatomic, strong) id<MTLBuffer> outputBuffer;

@property (nonatomic, strong) id<MTLBuffer> weightBuffer;

- (FBMetalCNNParameters*)cnn_parameters;

+ (id<MTLBuffer>)loadFilterWithImage:(const float*)weight_data
                        weightBuffer:(id<MTLBuffer>)weightBuffer
                             kernels:(NSUInteger)kernels
                       input_kernels:(NSUInteger)input_kernels
                       kernel_offset:(NSUInteger)kernel_offset
                       kernel_stride:(NSUInteger)kernel_stride
                            channels:(NSUInteger)channels
                               width:(NSUInteger)width
                              height:(NSUInteger)height
                          transposed:(bool)transposed
                             context:(MetalContext*)context;

- (void)loadFilterWithImage:(const float*)weight_data
                    kernels:(NSUInteger)kernels
              input_kernels:(NSUInteger)input_kernels
              kernel_offset:(NSUInteger)kernel_offset
              kernel_stride:(NSUInteger)kernel_stride
                   channels:(NSUInteger)channels
                      width:(NSUInteger)image_width
                     height:(NSUInteger)image_height
                 transposed:(bool)transposed;

+ (id<MTLBuffer>)loadDataWithImage:(const float*)imageData
                   imageDataBuffer:(id<MTLBuffer>)imageDataBuffer
                          channels:(NSUInteger)channels
                             width:(NSUInteger)width
                            height:(NSUInteger)height
                         padding_x:(NSUInteger)padding_x
                         padding_y:(NSUInteger)padding_y
                           context:(MetalContext*)context;

+ (instancetype)filterWithContext:(MetalContext*)context
                         channels:(NSUInteger)channels
                      kernel_size:(NSUInteger)kernel_size
                   constantValues:(FBMetalCNNConstantValues*)constantValues
                            width:(NSUInteger)width
                           height:(NSUInteger)height
                         stride_x:(NSUInteger)stride_x
                         stride_y:(NSUInteger)stride_y;

- (void) set_convolution_kernels:(NSUInteger)convolution_kernels;
@end
