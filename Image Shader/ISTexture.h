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

#ifndef __Image_Shader__ISTexture__
#define __Image_Shader__ISTexture__

#include <iostream>
#include <memory>
#include <set>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include "assert.h"

//TODO: split cache into ISTextureCache
class ISSingleton;
struct ISTexture {
    typedef const ISTexture* ISTextureRef;
    static void releasePool();
    ISTexture(GLuint width, GLuint height) : _width(width), _height(height), _name(0), _isValid(false), _strings(0), _dangling(0) { };
    ISTexture(const ISTexture& texture) : _width(texture._width), _height(texture._height), _name(texture._name), _isValid(texture._isValid), _strings(0), _dangling(0) { };
    virtual GLuint format() const = 0;
    virtual GLuint type() const = 0;
    
    virtual void setup();
    void attachToFramebuffer() const;
    void attachToFramebuffer(GLuint framebuffer) const;
    void detach() const;
    void ensureActiveTextureUnit(GLenum textureUnit) const;
    void bindToShader(GLuint shaderPosition, GLuint textureUnitOffset) const;
    void recycle() const;
    void split(size_t n) const;
    void glue() const;
    void terminate() const;
    size_t dangling() const { return _dangling; };
    virtual ~ISTexture();
    
    struct poolCompare {
        bool operator()(ISTextureRef lhs, ISTextureRef rhs) const;
    };
    
    GLuint width() const { return _width; };
    GLuint height() const { return _height; };
    GLuint name() const {
        assert(_isValid);
        if (_isValid) {
            return _name;
        }
        else {
            return 0;
        }
    };
    bool isValid() const { return _isValid; };
    virtual void deleteTexture() const;
    ISSingleton* asSingleton() const;
    template<class T>
    T& operator=(T rhs) {
        rhs.swap(*this);
        return *this;
    }
    void swap(ISTexture& other) throw();
protected:
    void tryRecycle() const;
    static GLenum _activeTextureUnit;
    static std::set<ISTextureRef, poolCompare> _texturePool; //TODO: See below
    GLuint _width;
    GLuint _height;
    mutable GLuint _name;
    mutable GLuint _isValid;
    mutable size_t _strings;
    mutable size_t _dangling;
};
typedef ISTexture::ISTextureRef ISTextureRef;



#endif /* defined(__Image_Shader__ISTexture__) */

//The lifetime of an ISTexture should exceed the GL texture it represents.
//GL textures are not deleted until all image processing is done, for performance
//reasons. Hence, none of the pointers in this cache will be deleted until
//a final cleanup sweep when we are done with the cache. The cache could be
//reused between draws so the texture cache might be around for the lifetime
//of the program
//At the end of a pipeline the only textures not on the cache will be those held
//by the pipeline value. To release all textures in a pipeline simply release the
//cache and the ISTexture's held in the value.
