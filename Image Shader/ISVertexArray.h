//
//  ISVertexArray.h
//  Image Shader
//
//  Created by William Talmadge on 6/5/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#ifndef __Image_Shader__ISVertexArray__
#define __Image_Shader__ISVertexArray__

#include <iostream>
#include <vector>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

typedef unsigned char uchar;

struct VertAttributeDescriptor
{
    GLuint position;
    GLuint size;
    GLenum type;
    GLuint length();
};

struct ISVertexArray {
    ISVertexArray();
    ~ISVertexArray();
    
    GLuint vertexSize();
    void addFloatAttribute(GLuint index, GLuint size);
    void upload(const uchar* data, size_t length);
    template<class T>
    void upload(const std::vector<T>& data) {
        upload(reinterpret_cast<const uchar*>(data.data()), data.size()*sizeof(T));
    }
    void draw() const;
    void deleteArray() const;
protected:
    size_t geometrySize;
    GLuint vertexArray;
    GLuint vertexBuffer;
    GLenum drawType;
    GLuint vertexCount;
    std::vector<VertAttributeDescriptor> attributes;
    mutable bool isSetup;
};

#endif /* defined(__Image_Shader__ISVertexArray__) */
