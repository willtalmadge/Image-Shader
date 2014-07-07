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

#include "FFTGaussianFilter.h"
#include <string>
const std::string FFTGaussianFilter::fragShader = SHADER_STRING
(
 varying highp vec2 i;
 varying highp vec2 r;
 
 uniform sampler2D tex;
 uniform highp float wSq; //This must be the real image width, not width of one complex channel
 uniform highp float hSq;
 uniform highp float var;
 
 const highp float k2PI = 6.28318530718;
 void main()
{
	highp vec4 val = texture2D(tex, i);
    highp float g = exp(-var*(r.x*r.x/wSq + r.y*r.y/hSq)/2.0);
	highp vec4 result;
	result.rgb = g*val.rgb;
	result.a = 1.0;
    gl_FragColor = result;
}
 );

const std::string FFTGaussianFilter::vertShader = SHADER_STRING
(
 attribute vec4 positionIn;
 attribute vec2 iIn;
 attribute vec2 rIn;
 
 varying vec2 i;
 varying vec2 r;
 
 uniform mat4 orthoMatrix;
 void main()
{
    i = iIn;
	r = rIn;
	gl_Position = orthoMatrix*positionIn;
}
 );