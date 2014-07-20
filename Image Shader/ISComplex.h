//  Created by William Talmadge on 6/14/14.
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

#ifndef __Image_Shader__ISComplex__
#define __Image_Shader__ISComplex__

#include <iostream>
#include <memory>
#include "ISTextureTuple.h"

struct ISComplexBindable {
    virtual GLuint realBindingTarget() const = 0;
    virtual GLuint imagBindingTarget() const = 0;
};

struct ISComplex : ISTextureTuple {
    template<class T>
    void setup(GLuint width, GLuint height);
    void setup(std::unique_ptr<ISSingleton>& real, std::unique_ptr<ISSingleton>& imag);
    ISTextureRef getReal() const;
    ISTextureRef getImag() const;
    void setReal(ISTextureRef real) { _elements[0] = real; }
    void setImag(ISTextureRef imag) { _elements[1] = imag; }
    void bind(const ISComplexBindable* drawable);
    GLuint textureUnitsUsed() const { return 2; };
};
template<class T>
void ISComplex::setup(GLuint width, GLuint height) {
    _elements = {NULL, NULL};
    ISTextureRef real = T::make(width, height);
    ISTextureRef imag = T::make(width, height);
    setReal(real);
    setImag(imag);
}
#endif /* defined(__Image_Shader__ISComplex__) */
