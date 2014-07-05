//
//  ISTextureTuple.h
//  Image Shader
//
//  Created by William Talmadge on 6/1/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

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
protected:
    std::vector<ISTextureRef> _elements;
};
#endif /* defined(__Image_Shader__ISTextureTuple__) */
