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

#include "ISRemap.h"

using namespace std;

const string ISRemap::fragShader = SHADER_STRING
(
 varying highp vec2 r;
 uniform highp sampler2D texture;
 uniform highp sampler2D remap;
 void main()
 {
     highp vec4 l = texture2D(remap, r);
     highp vec4 v = texture2D(texture, l.xy);
     gl_FragColor = v;
 }
 );
const string ISRemap::vertShader = SHADER_STRING
(
 attribute vec4 rWrite;
 attribute highp vec2 rRead;
 
 varying highp vec2 r;
 void main()
 {
     gl_Position = orthoMatrix*rWrite;
     r = rRead;
 }
 );