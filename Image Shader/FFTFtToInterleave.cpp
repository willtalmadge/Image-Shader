//
//  FFTFtToInterleave.cpp
//  Image Shader
//
//  Created by William Talmadge on 7/1/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#include "FFTFtToInterleave.h"

const std::string FFTFtToInterleave::fragShaderRe = SHADER_STRING
(
 varying highp vec2 n;
 varying highp vec2 N2mn; //N/2 - n
 
 uniform sampler2D inputRe;
 uniform sampler2D inputIm;
 uniform sampler2D phaseTable;
 
 void main()
 {
     highp vec4 re = texture2D(inputRe, n);
     highp vec4 ren2 = texture2D(inputRe, N2mn);
     highp vec4 im = texture2D(inputIm, n);
     highp vec4 imn2 = texture2D(inputIm, N2mn);
     highp vec4 p = texture2D(phaseTable, n);
     highp vec4 result;
     result.rgb = 0.5*(re.rgb - p.y*re.rgb + ren2.rgb + p.y*ren2.rgb - (im.rgb + imn2.rgb)*p.x);
     result.a = 1.0;
     gl_FragColor = result;
 }
 );
const std::string FFTFtToInterleave::fragShaderIm = SHADER_STRING
(
 varying highp vec2 n;
 varying highp vec2 N2mn;
 
 uniform sampler2D inputRe;
 uniform sampler2D inputIm;
 uniform sampler2D phaseTable;
 
 void main()
 {
     highp vec4 re = texture2D(inputRe, n);
     highp vec4 ren2 = texture2D(inputRe, N2mn);
     highp vec4 im = texture2D(inputIm, n);
     highp vec4 imn2 = texture2D(inputIm, N2mn);
     highp vec4 p = texture2D(phaseTable, n);
     highp vec4 result;
     result.rgb = 0.5*(im.rgb - imn2.rgb - (im.rgb + imn2.rgb)*p.y + p.x*(re.rgb - ren2.rgb));
     result.a = 1.0;
     gl_FragColor = result;
 }
 );

const std::string FFTFtToInterleave::vertShader = SHADER_STRING
(
 attribute vec4 positionIn;
 attribute vec2 nIn;
 attribute vec2 N2mnIn;
 
 varying vec2 n;
 varying vec2 N2mn;
 
 uniform mat4 orthoMatrix;
 
 void main()
 {
     gl_Position = orthoMatrix*positionIn;
     n = nIn;
     N2mn = N2mnIn;
 }
 
 );