//
//  FFTInterleaveToFtNdc.cpp
//  Image Shader
//
//  Created by William Talmadge on 6/29/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#include "FFTInterleaveToFtNdc.h"

const std::string FFTInterleaveToFtNdc::fragShaderRe = SHADER_STRING
(
 varying highp vec2 n;
 uniform sampler2D inputRe;
 uniform sampler2D inputIm;
 
 void main()
{
    highp vec4 re = texture2D(inputRe, n);
    highp vec4 im = texture2D(inputIm, n);
    highp vec4 result;
    result.rgb = re.rgb + im.rgb;
    result.a = 1.0;
    gl_FragColor = result;
}
 );
const std::string FFTInterleaveToFtNdc::fragShaderIm = SHADER_STRING
(
 varying highp vec2 n;
 uniform sampler2D inputRe;
 uniform sampler2D inputIm;
 
 void main()
{
    highp vec4 re = texture2D(inputRe, n);
    highp vec4 im = texture2D(inputIm, n);
    highp vec4 result;
    result.rgb = re.rgb - im.rgb;
    result.a = 1.0;
    gl_FragColor = result;
}
 );

const std::string FFTInterleaveToFtNdc::vertShader = SHADER_STRING
(
 attribute vec4 positionIn;
 attribute vec2 nIn;
 
 varying vec2 n;
 
 uniform mat4 orthoMatrix;
 
 void main()
{
    gl_Position = orthoMatrix*positionIn;
    n = nIn;
}
 
 );