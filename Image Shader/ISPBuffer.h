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

#ifndef __Image_Shader__ISPBuffer__
#define __Image_Shader__ISPBuffer__

#include <iostream>
#include "ISTexture.h"
#include <CoreVideo/CoreVideo.h>

struct ISPBuffer : public ISTexture {
    typedef const ISPBuffer* ISPBufferRef;
    ISPBuffer(GLuint width, GLuint height) : ISTexture(width, height), _baseAddress(NULL), _pBuffer(NULL), _texture(NULL), _textureCache(NULL) { }
    void setup();
    void deleteTexture() const;
    void* baseAddress() const;
    void bindBaseAddress() const;
    void unbindBaseAddress() const;

protected:
    //How to pass in the context? It is a core video object. Use a static variable and assert it to ensure it is set. We require a buffered pipeline. Make it depend on the bufferable pipeline static context variable.
    //How to select the texture init type? Base class knows how to translate type() result.
    mutable void* _baseAddress;
    CVPixelBufferRef _pBuffer;
    CVOpenGLESTextureRef _texture;
    CVOpenGLESTextureCacheRef _textureCache;
};
typedef ISPBuffer::ISPBufferRef ISPBufferRef;
#endif /* defined(__Image_Shader__ISPBuffer__) */
