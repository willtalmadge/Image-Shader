//
//  ISTexture.h
//  Image Shader
//
//  Created by William Talmadge on 6/1/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

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
    static void releaseCache();
    ISTexture(GLuint width, GLuint height) : _width(width), _height(height), _name(0), _isValid(false) { };
    ISTexture(const ISTexture& texture) : _width(texture._width), _height(texture._height), _name(texture._name), _isValid(texture._isValid) { };
    virtual GLuint format() const = 0;
    virtual GLuint type() const = 0;
    
    void setup();
    void attachToFramebuffer() const;
    void attachToFramebuffer(GLuint framebuffer) const;
    void detach() const;
    void ensureActiveTextureUnit(GLenum textureUnit) const;
    void bindToShader(GLuint shaderPosition, GLuint textureUnitOffset) const;
    void recycle() const;
    virtual ~ISTexture();
    
    struct cacheCompare {
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
    void deleteTexture() const;
    ISSingleton* asSingleton() const;
    template<class T>
    T& operator=(T rhs) {
        rhs.swap(*this);
        return *this;
    }
    void swap(ISTexture& other) throw();
protected:
    static GLenum _activeTextureUnit;
    static std::set<ISTextureRef, cacheCompare> _textureCache; //TODO: See below
    GLuint _width;
    GLuint _height;
    mutable GLuint _name;
    mutable GLuint _isValid;
};
typedef ISTexture::ISTextureRef ISTextureRef;

struct ISURe8Rgba : public ISTexture {
    ISURe8Rgba(GLuint width, GLuint height) : ISTexture(width, height) { };
    virtual GLuint format() const { return GL_RGBA; };
    virtual GLuint type() const { return GL_UNSIGNED_BYTE; };
    static const ISURe8Rgba* make(GLuint width, GLuint height) {
        ISURe8Rgba* result = new ISURe8Rgba(width, height);
        result->setup();
        return result;
    }
    static const ISURe8Rgba* fromExisting(GLuint name, GLuint width, GLuint height, GLenum type);
};
struct ISRe16Rgba : public ISTexture {
    ISRe16Rgba(GLuint width, GLuint height) : ISTexture(width, height) { };
    virtual GLuint format() const { return GL_RGBA; };
    virtual GLuint type() const { return GL_HALF_FLOAT_OES; };
    static const ISRe16Rgba* make(GLuint width, GLuint height) {
        ISRe16Rgba* result = new ISRe16Rgba(width, height);
        result->setup();
        return result;
    }
    static const ISRe16Rgba* fromExisting(GLuint name, GLuint width, GLuint height, GLenum type);
};



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
