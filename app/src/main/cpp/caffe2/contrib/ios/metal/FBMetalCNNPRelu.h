// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#import "arm_neon_support.h"
#import "MetalImageFilter.h"

static constexpr size_t kThreadGroupSize_x = 4;
static constexpr size_t kThreadGroupSize_y = 8;

@interface FBMetalCNNPRelu : MetalImageFilter

@property (nonatomic, strong) id<MTLBuffer> dataBuffer;
@property (nonatomic, strong) id<MTLBuffer> outputBuffer;
@property (nonatomic, strong) id<MTLBuffer> weightBuffer;

+ (instancetype)filterWithContext:(MetalContext*)context
                   constantValues:(FBMetalCNNConstantValues*)constantValues
                            width:(NSUInteger)width
                           height:(NSUInteger)height;

@end
