//
//  FFTPhaseTable.h
//  Image Shader
//
//  Created by William Talmadge on 7/1/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#ifndef __Image_Shader__FFTPhaseTable__
#define __Image_Shader__FFTPhaseTable__

#include <iostream>
#include <unordered_map>
#include "ISShaderProgram.h"
#include "ISTexture.h"
#include <GLKit/GLKMatrix4.h>

struct FFTPhaseTable {
    static void preparePhaseTable(GLuint sigSize, int sign);
    static ISTextureRef renderPhaseTable(GLuint sigSize, int sign);
    static ISTextureRef getPhaseTable(GLuint sigSize, int sign);
    static void bindPhaseTable(GLuint textureUnit, GLuint sigSize, int sign);
    static void releaseCache();
    
    static std::unordered_map<int, ISTextureRef> _phaseTableCache;
};
#endif /* defined(__Image_Shader__FFTPhaseTable__) */
