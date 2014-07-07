//  Created by William Talmadge on 6/12/14.
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

#include "FFTPermute.h"

const std::string FFTPermute::fragShader = SHADER_STRING
(
 varying highp vec2 textureCoordinate;
 uniform sampler2D inputImageTexture;
 
 void main()
 {
     highp vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);
     gl_FragColor = textureColor;
 }
 );

const std::string FFTPermute::vertShaderCols = SHADER_STRING
(
 attribute vec4 positionIn;
 attribute highp float texCoordVIn;
 attribute highp float sourceIndexIn;
 
 varying vec2 textureCoordinate;
 
 uniform mat4 orthoMatrix;
 uniform highp float stride;
 uniform highp float offset;
 uniform highp float targetSize;
 
 void main()
 {
     gl_Position = orthoMatrix*positionIn;
     highp float u = (stride*sourceIndexIn + offset + 0.5)/targetSize;
     textureCoordinate = vec2(u, texCoordVIn);
 }
 );
const std::string FFTPermute::vertShaderRows = SHADER_STRING
(
 attribute vec4 positionIn;
 attribute highp float texCoordVIn;
 attribute highp float sourceIndexIn;
 
 varying vec2 textureCoordinate;
 
 uniform mat4 orthoMatrix;
 uniform highp float stride;
 uniform highp float offset;
 uniform highp float targetSize;
 
 void main()
 {
     gl_Position = orthoMatrix*positionIn;
     highp float u = (stride*sourceIndexIn + offset + 0.5)/targetSize;
     textureCoordinate = vec2(texCoordVIn,u);
 }
 );