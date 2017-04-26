// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#import "arm_neon_support.h"
#import "MetalImageFilter.h"

static constexpr size_t kThreadGroupSize_x = 1;
static constexpr size_t kThreadGroupSize_y = 1;

@interface FBMetalCNNNoOp: MetalImageFilter

+ (instancetype)filterWithContext:(MetalContext*)context
                            width:(NSUInteger)width
                           height:(NSUInteger)height;

@end
