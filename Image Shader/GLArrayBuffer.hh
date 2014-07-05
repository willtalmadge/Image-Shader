//
//  GLArrayBuffer.h
//  Image Shader
//
//  Created by William Talmadge on 4/26/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <GLKit/GLKit.h>
#import <vector>

struct GLVertAttributeDescriptor
{
    GLuint position;
    GLuint size;
    GLenum type;
    GLuint length();
};


@interface GLArrayBuffer : NSObject

@property GLuint vertexArray;
@property GLuint vertexBuffer;
@property GLenum drawType;
@property GLuint vertexCount;
@property std::vector<GLVertAttributeDescriptor> attributes;
@property NSData* geometryData;
@property BOOL isSetup;

- (void)addFloatAttribute:(GLuint)index size:(GLuint)size;
- (id)initWithDataToCopy:(const void*)data length:(NSUInteger)length;
- (void)setupGL;
- (void)draw;
- (void)cleanup;
- (void)bind;
@end
