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

class ISPipeline;
struct ISTextureTuple {
    void join(ISTextureTuple* previous);
    void deleteTextures();
    virtual GLuint textureUnitsUsed() const = 0;
    virtual ~ISTextureTuple();
    ISPipeline pipeline();
    void map(std::function<void (ISTextureRef)> f); //TODO: consider making this only accessible to ISPipeline
protected:
    std::vector<ISTextureRef> _elements;
};
#endif /* defined(__Image_Shader__ISTextureTuple__) */
