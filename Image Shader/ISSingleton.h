//
//  ISSingleton.h
//  Image Shader
//
//  Created by William Talmadge on 6/4/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#ifndef __Image_Shader__ISSingleton__
#define __Image_Shader__ISSingleton__

#include <iostream>
#include <memory>
#include "ISTextureTuple.h"
#include "assert.h"
#include "Macro.hpp"

struct ISSingletonBindable {
    virtual GLuint textureBindingTarget() const = 0;
};

struct ISSingleton : ISTextureTuple {
    template<class T>
    void setup(GLuint width, GLuint height) {
        ISTextureRef newTexture = T::make(width, height);
        setTexture(newTexture);
    }
    void setup(std::unique_ptr<ISSingleton>& input);
    ISTextureRef getTexture() const;
    void setTexture(ISTextureRef texture);
    void bind(const ISSingletonBindable* drawable) const;
    void attach() const;
    GLuint textureUnitsUsed() const { return 1; };
};

#endif /* defined(__Image_Shader__ISSingleton__) */
