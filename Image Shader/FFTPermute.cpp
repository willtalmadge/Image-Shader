//
//  FFTPermuteColumns.cpp
//  Image Shader
//
//  Created by William Talmadge on 6/12/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

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