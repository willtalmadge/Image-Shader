//  Created by William Talmadge on 6/1/14.
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

#ifndef __Image_Shader__ISTextureTuple__
#define __Image_Shader__ISTextureTuple__

#include <iostream>
#include <vector>
#include "ISTexture.h"
#include "ISRect.h"

class ISPipeline;
struct ISTextureTuple {
    void join(ISTextureTuple* previous);
    void deleteTextures();
    virtual GLuint textureUnitsUsed() const = 0;
    virtual ISSize size() const = 0;
    virtual ~ISTextureTuple();
    ISPipeline pipeline();
    void map(std::function<void (ISTextureRef)> f); //TODO: consider making this only accessible to ISPipeline
//FIXME: restore protected:
    std::vector<ISTextureRef> _elements;
};

#define IS_2TUPLE_HEADER(TUPLE_NAME, TEXTURE1_NAME, TEXTURE2_NAME) \
struct TUPLE_NAME##Bindable { \
    virtual GLuint TEXTURE1_NAME##BindingTarget() const = 0; \
    virtual GLuint TEXTURE2_NAME##BindingTarget() const = 0; \
};\
struct TUPLE_NAME : ISTextureTuple { \
    template<class T> \
    void setup(GLuint width, GLuint height); \
    void setup(std::unique_ptr<ISSingleton>& TEXTURE1_NAME, std::unique_ptr<ISSingleton>& TEXTURE2_NAME); \
void setup(ISTextureRef TEXTURE1_NAME, ISTextureRef TEXTURE2_NAME);\
    ISTextureRef get_##TEXTURE1_NAME() const; \
    ISTextureRef get_##TEXTURE2_NAME() const; \
    void set_##TEXTURE1_NAME(ISTextureRef TEXTURE1_NAME) { _elements[0] = TEXTURE1_NAME; } \
    void set_##TEXTURE2_NAME(ISTextureRef TEXTURE2_NAME) { _elements[1] = TEXTURE2_NAME; } \
    void bind(const TUPLE_NAME##Bindable* drawable); \
    GLuint textureUnitsUsed() const { return 2; }; \
    ISSize size() const; \
}; \
template<class T> \
void TUPLE_NAME::setup(GLuint width, GLuint height) { \
    _elements = {NULL, NULL}; \
    ISTextureRef TEXTURE1_NAME = T::make(width, height); \
    ISTextureRef TEXTURE2_NAME = T::make(width, height); \
    set_##TEXTURE1_NAME(TEXTURE1_NAME); \
    set_##TEXTURE2_NAME(TEXTURE2_NAME); \
}

#define IS_2TUPLE_IMPLEMENTATION(TUPLE_NAME, TEXTURE1_NAME, TEXTURE2_NAME) \
void TUPLE_NAME::setup(std::unique_ptr<ISSingleton>& TEXTURE1_NAME, std::unique_ptr<ISSingleton>& TEXTURE2_NAME) { \
_elements = {NULL, NULL}; \
set_##TEXTURE1_NAME(TEXTURE1_NAME->getTexture()); \
set_##TEXTURE2_NAME(TEXTURE2_NAME->getTexture()); \
} \
void TUPLE_NAME::setup(ISTextureRef TEXTURE1_NAME, ISTextureRef TEXTURE2_NAME) { \
_elements = {NULL, NULL}; \
set_##TEXTURE1_NAME(TEXTURE1_NAME); \
set_##TEXTURE2_NAME(TEXTURE2_NAME); \
} \
ISTextureRef TUPLE_NAME::get_##TEXTURE1_NAME() const { \
    return _elements[0]; \
} \
ISTextureRef TUPLE_NAME::get_##TEXTURE2_NAME() const { \
    return _elements[1]; \
} \
void TUPLE_NAME::bind(const TUPLE_NAME##Bindable* drawable) { \
    ISTextureRef TEXTURE1_NAME = get_##TEXTURE1_NAME(); \
    ISTextureRef TEXTURE2_NAME = get_##TEXTURE2_NAME(); \
    TEXTURE1_NAME->bindToShader(drawable->TEXTURE1_NAME##BindingTarget(), 0); \
    TEXTURE2_NAME->bindToShader(drawable->TEXTURE2_NAME##BindingTarget(), 1); \
} \
ISSize TUPLE_NAME::size() const { \
    auto size = ISSize().width(get_##TEXTURE1_NAME()->width()) \
    .height(get_##TEXTURE1_NAME()->height()); \
    return size; \
}

#endif /* defined(__Image_Shader__ISTextureTuple__) */
