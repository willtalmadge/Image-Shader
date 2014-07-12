//
//  ISURe8Rgba.h
//  Image Shader
//
//  Created by William Talmadge on 7/12/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#ifndef __Image_Shader__ISURe8Rgba__
#define __Image_Shader__ISURe8Rgba__

#include <iostream>
#include "ISTexture.h"

template<class TextureT = ISTexture>
struct ISURe8Rgba : public TextureT {
    //TextureT should be ISTexture or a child class
    ISURe8Rgba(GLuint width, GLuint height) : TextureT(width, height) { };
    virtual GLuint format() const { return GL_RGBA; };
    virtual GLuint type() const { return GL_UNSIGNED_BYTE; };
    static const ISURe8Rgba* make(GLuint width, GLuint height) {
        ISURe8Rgba* result = new ISURe8Rgba(width, height);
        result->setup();
        return result;
    }
    static const ISURe8Rgba* fromExisting(GLuint name, GLuint width, GLuint height, GLenum type);
};
template<class TextureT>
const ISURe8Rgba<TextureT>* ISURe8Rgba<TextureT>::fromExisting(GLuint name, GLuint width, GLuint height, GLenum type)
{
    //These methods are problematic when rebasing to a ISTexture child
    //setup is never called so the object will be in an undefined state.
    //TODO: There is probably a better way to handle this.
    assert(glIsTexture(name));
    assert(type == GL_UNSIGNED_BYTE);
    ISURe8Rgba* result = new ISURe8Rgba(width, height);
    result->_name = name;
    result->_isValid = true;
    return result;
}
#endif /* defined(__Image_Shader__ISURe8Rgba__) */
