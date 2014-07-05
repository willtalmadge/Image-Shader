//
//  ISComplex.h
//  Image Shader
//
//  Created by William Talmadge on 6/14/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

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

#endif /* defined(__Image_Shader__ISComplex__) */
