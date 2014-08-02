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

#include "FSTPrecondition.h"

using namespace std;

string FSTPrecondition::fragShader = SHADER_STRING
(
 varying highp vec2 j;
 varying highp vec2 Nmj;
 varying highp vec2 stj;
 
 uniform highp sampler2D tex;
 uniform highp sampler2D phaseTable;
 uniform highp float scale;
 void main()
{
	highp float st = texture2D(phaseTable, stj).y;
	highp vec4 fj = texture2D(tex, j);
	highp vec4 fNmj = texture2D(tex, Nmj);
	
	highp vec4 result;
	result.rgb = st*(fj.rgb + fNmj.rgb) + 0.5*(fj.rgb - fNmj.rgb);
	result.a = 1.0;
	
	gl_FragColor = scale*result;
}
);

string FSTPrecondition::vertShader = SHADER_STRING
(
 attribute highp vec4 jWrite;
 attribute highp vec2 jRead;
 attribute highp vec2 NmjRead;
 attribute highp vec2 stjRead;
 
 varying highp vec2 j;
 varying highp vec2 Nmj;
 varying highp vec2 stj;
 
 void main()
{
	gl_Position = orthoMatrix*jWrite;
	j = jRead;
	Nmj = NmjRead;
	stj = stjRead;
}

);