//
//  ISRe16Rgba.h
//  Image Shader
//
//  Created by William Talmadge on 7/12/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#ifndef __Image_Shader__ISRe16Rgba__
#define __Image_Shader__ISRe16Rgba__

#include <iostream>
#include "ISTexture.h"

template<class TextureT = ISTexture>
struct ISRe16Rgba : public TextureT {
    ISRe16Rgba(GLuint width, GLuint height) : TextureT(width, height) { };
    virtual GLuint format() const { return GL_RGBA; };
    virtual GLuint type() const { return GL_HALF_FLOAT_OES; };
    static const ISRe16Rgba* make(GLuint width, GLuint height) {
        ISRe16Rgba* result = new ISRe16Rgba(width, height);
        result->setup();
        return result;
    }
    static const ISRe16Rgba* fromExisting(GLuint name, GLuint width, GLuint height, GLenum type);
};
template<class TextureT>
const ISRe16Rgba<TextureT>* ISRe16Rgba<TextureT>::fromExisting(GLuint name, GLuint width, GLuint height, GLenum type)
{
    assert(glIsTexture(name));
    assert(type == GL_HALF_FLOAT_OES);
    ISRe16Rgba* result = new ISRe16Rgba(width, height);
    result->_name = name;
    result->_isValid = true;
    return result;
}
#endif /* defined(__Image_Shader__ISRe16Rgba__) */
