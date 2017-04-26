// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#import "MetalContext.h"
#import "FBMetalCNNConstantValues.h"

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

@protocol MTLBuffer, MTLComputeCommandEncoder, MTLComputePipelineState;

@interface MetalImageFilter : NSObject {
  int threadGroupSize_x;
  int threadGroupSize_y;

  int stride_x;
  int stride_y;
}

@property (nonatomic, strong) MetalContext*               context;
@property (nonatomic, strong) id<MTLBuffer>               parametersBuffer;
@property (nonatomic, strong) id<MTLComputePipelineState> pipeline;

@property (atomic, strong) MTLTextureDescriptor* outputTextureDescriptor;

- (instancetype)initWithFunctionName:(NSString*)functionName
                             context:(MetalContext*)context
                      constantValues:(FBMetalCNNConstantValues*)constantValues;

- (void)configureArgumentTableWithCommandEncoder:(id<MTLComputeCommandEncoder>)commandEncoder
                              weightBufferOffset:(NSInteger)weightBufferOffset
                              outputBufferOffset:(NSInteger)outputBufferOffset;

typedef void (^MetalImageFilterHandler)(id<MTLCommandBuffer>);

- (void)applyFilter:(void (^)(NSError*))completionHandler;

- (void) applyFilter:(void(^)(NSError*))completionHandler
  weightBufferOffset:(NSInteger)weightBufferOffset
  outputBufferOffset:(NSInteger)outputBufferOffset;

@end
