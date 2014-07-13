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
