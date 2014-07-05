//
//  GLArrayBuffer.m
//  Image Shader
//
//  Created by William Talmadge on 4/26/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#import "GLArrayBuffer.hh"

using namespace std;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
GLuint GLVertAttributeDescriptor::length()
{
    switch (type) {
        case GL_FLOAT:
            return size*sizeof(GLfloat);
            break;
        default:
            return 0;
            break;
    }
}
@implementation GLArrayBuffer

- (id)initWithDataToCopy:(const void*)data length:(NSUInteger)length
{
    self = [super init];
    if (self) {
        _isSetup = NO;
        _drawType = GL_TRIANGLES;
        _geometryData = [NSData dataWithBytes:data length:length];
    }
    return self;
}

- (GLuint)vertexSize
{
    GLuint offset = 0;
    for (GLVertAttributeDescriptor attrDesc : _attributes)
    {
        offset += attrDesc.length();
    }
    return offset;
}

- (void)addFloatAttribute:(GLuint)index size:(GLuint)size
{
    GLVertAttributeDescriptor newAttr;
    newAttr.position = index;
    newAttr.size = size;
    newAttr.type = GL_FLOAT;
    _attributes.push_back(newAttr);
    _vertexCount = (GLuint)[_geometryData length]/[self vertexSize];
}

- (void)setupGL
{
    if (!_isSetup) {
        glGenVertexArraysOES(1, &_vertexArray);
        glBindVertexArrayOES(_vertexArray);
        
        glGenBuffers(1, &_vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, [_geometryData length], [_geometryData bytes], GL_STATIC_DRAW);
        
        GLuint stride = [self vertexSize];
        GLuint offset = 0;
        for (GLVertAttributeDescriptor attrDesc : _attributes) {
            NSLog(@"glVertexAttribPointer(%d,%d,%d,GL_FALSE,%d,BUFFER_OFFSET(%d))",
                  attrDesc.position, attrDesc.size,
                  attrDesc.type,
                  stride, offset);
            glEnableVertexAttribArray(attrDesc.position);
            glVertexAttribPointer(attrDesc.position, attrDesc.size,
                                  attrDesc.type, GL_FALSE,
                                  stride, BUFFER_OFFSET(offset));
            offset += attrDesc.length();
        }
        glBindVertexArrayOES(0);
        _isSetup = YES;
        GLenum error = glGetError();
        if (error) {
            NSLog(@"GLArrayBuffer setup error %d", error);
        }
    }
}
- (void)bind
{
    glBindVertexArrayOES(_vertexArray);
}
- (void)draw
{
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE){
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArrayOES(_vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, _vertexCount);
        GLenum error = glGetError();
        if (error) {
            NSLog(@"GLArrayBuffer draw error %d", error);
        }
    }
    
    else {
        NSLog(@"GL_FRAMEBUFFER incomplete");
    }
}

- (void)cleanup
{
    if (_isSetup) {
        glDeleteBuffers(1, &_vertexBuffer);
        glDeleteVertexArraysOES(1, &_vertexArray);
        _isSetup = NO;
    }
}

- (void)dealloc
{
    [self cleanup];
}

@end
