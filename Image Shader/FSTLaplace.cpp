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

//TODO: use a phase table to remove calls to cos()
#include "FSTLaplace.h"
const std::string FSTLaplace::fragShader = SHADER_STRING
(
 varying highp vec2 rTex;
 varying highp vec2 r;
 
 uniform highp sampler2D inputImageTexture;
 
 const highp float pi = 3.14159265359;
 void main()
{
    highp vec4 v = texture2D(inputImageTexture, rTex);
    highp float k = 1.0/(2.0*cos(r.x*pi) + 2.0*cos(r.y*pi) - 4.0);
    gl_FragColor = k*v;
}
 );

const std::string FSTLaplace::vertShader = SHADER_STRING
(
 attribute vec4 rWrite;
 attribute vec2 rRead;
 attribute vec2 rMath;
 
 varying vec2 rTex;
 varying vec2 r;
 
 void main()
{
    gl_Position = orthoMatrix*rWrite;
    rTex = rRead;
    r = rMath;
}
 
 );