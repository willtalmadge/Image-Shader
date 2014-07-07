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

#include "ISComplex.h"
#include "ISSingleton.h"

template<class T>
void ISComplex::setup(GLuint width, GLuint height) {
    _elements = {NULL, NULL};
    ISTextureRef real = T::make(width, height);
    ISTextureRef imag = T::make(width, height);
    setReal(real);
    setImag(imag);
}
void ISComplex::setup(std::unique_ptr<ISSingleton>& real, std::unique_ptr<ISSingleton>& imag) {
    _elements = {NULL, NULL};
    setReal(real->getTexture());
    setImag(imag->getTexture());
}
ISTextureRef ISComplex::getReal() const {
    return _elements[0];
}
ISTextureRef ISComplex::getImag() const {
    return _elements[1];
}
void ISComplex::bind(const ISComplexBindable* drawable) {
    ISTextureRef real = getReal();
    ISTextureRef imag = getImag();
    real->bindToShader(drawable->realBindingTarget(), 0);
    imag->bindToShader(drawable->imagBindingTarget(), 1);
}