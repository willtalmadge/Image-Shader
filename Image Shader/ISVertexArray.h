/*
 Copyright (C) 2014  William B. Talmadge
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

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
