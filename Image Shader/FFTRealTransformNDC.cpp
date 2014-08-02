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

#include "FFTRealTransformNDC.h"

const std::string FFTRealTransformNDC::fragShaderRe = SHADER_STRING
(
 varying highp vec2 n;
 uniform sampler2D inputRe;
 uniform sampler2D inputIm;
 uniform highp float c;
 
 void main()
{
    highp vec4 re = texture2D(inputRe, n);
    highp vec4 im = texture2D(inputIm, n);
    highp vec4 result;
    result.rgb = c*(re.rgb + im.rgb);
    result.a = 1.0;
    gl_FragColor = result;
}
 );
const std::string FFTRealTransformNDC::fragShaderIm = SHADER_STRING
(
 varying highp vec2 n;
 uniform sampler2D inputRe;
 uniform sampler2D inputIm;
 uniform highp float c;
 
 void main()
{
    highp vec4 re = texture2D(inputRe, n);
    highp vec4 im = texture2D(inputIm, n);
    highp vec4 result;
    result.rgb = c*(re.rgb - im.rgb);
    result.a = 1.0;
    gl_FragColor = result;
}
 );

const std::string FFTRealTransformNDC::vertShader = SHADER_STRING
(
 attribute vec4 positionIn;
 attribute vec2 nIn;
 
 varying vec2 n;
 
 void main()
{
    gl_Position = orthoMatrix*positionIn;
    n = nIn;
}
 
 );