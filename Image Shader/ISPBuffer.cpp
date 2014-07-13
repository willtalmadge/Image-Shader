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

#include "ISPBuffer.h"
CVEAGLContext ISPBuffer::context = NULL;

void ISPBuffer::setup()
{
    
    CFDictionaryRef empty;
    CFMutableDictionaryRef attrs;
    empty = CFDictionaryCreate(kCFAllocatorDefault,
                               NULL,
                               NULL,
                               0,
                               &kCFTypeDictionaryKeyCallBacks,
                               &kCFTypeDictionaryValueCallBacks);
    attrs = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                      1,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks);
    
    CFDictionarySetValue(attrs,
                         kCVPixelBufferIOSurfacePropertiesKey,
                         empty);
    OSType cvType;
    if (type() == GL_UNSIGNED_BYTE) {
        cvType = kCVPixelFormatType_32RGBA;
    } else if (type() == GL_HALF_FLOAT_OES) {
        cvType = kCVPixelFormatType_64RGBAHalf;
    } else {
        assert(false); //You've used a texture type that needs a type added here.
    }
    CVPixelBufferCreate(kCFAllocatorDefault, _width, _height,
                        cvType,
                        attrs,
                        &_pBuffer);
    
    CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, context, NULL, &_textureCache);
    CVOpenGLESTextureCacheCreateTextureFromImage (
                                                  kCFAllocatorDefault,
                                                  _textureCache,
                                                  _pBuffer,
                                                  NULL, // texture attributes
                                                  GL_TEXTURE_2D,
                                                  format(), // opengl format
                                                  _width,
                                                  _height,
                                                  GL_BGRA, // native iOS format
                                                  type(),
                                                  0,
                                                  &_texture);
    _name = CVOpenGLESTextureGetName(_texture);
    glBindTexture(GL_TEXTURE_2D, _name);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
void ISPBuffer::deleteTexture() const
{
    //TODO: after the pool is cleared, call CVOpenGLESTextureCacheFlush
    assert(_pBuffer != NULL);
    assert(_textureCache != NULL);
    assert(_texture != NULL);
    CVPixelBufferRelease(_pBuffer);
    CFRelease(_textureCache);
    CFRelease(_texture);
}
void* ISPBuffer::baseAddress()
{
    assert(_baseAddress); //You are asking for the base address without locking it
    return _baseAddress;
}
void ISPBuffer::bindBaseAddress()
{
    glFinish();
    if (kCVReturnSuccess == CVPixelBufferLockBaseAddress(_pBuffer, 0)) {
        _baseAddress = CVPixelBufferGetBaseAddress(_pBuffer);
    }
}
void ISPBuffer::unbindBaseAddres()
{
    CVPixelBufferUnlockBaseAddress(_pBuffer, 0);
    _baseAddress = NULL;
}