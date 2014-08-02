//  Created by William Talmadge on 6/4/14.
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
    ISSize size() const;
};

#endif /* defined(__Image_Shader__ISSingleton__) */
