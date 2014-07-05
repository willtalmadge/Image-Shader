//
//  ISComplex.cpp
//  Image Shader
//
//  Created by William Talmadge on 6/14/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

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