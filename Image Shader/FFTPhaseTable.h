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
    static ISTextureRef renderPhaseTable(GLuint sigSize, GLfloat period, int sign);
    static ISTextureRef getPhaseTable(GLuint sigSize, int sign);
    static ISTextureRef getPhaseTable(GLuint sigSize, GLfloat period, int sign);
    static void bindPhaseTable(GLuint textureUnit, GLuint sigSize, int sign);
    static void releaseCache();
    
    static std::unordered_map<std::pair<int, float>, ISTextureRef> _phaseTableCache;
};
#endif /* defined(__Image_Shader__FFTPhaseTable__) */
