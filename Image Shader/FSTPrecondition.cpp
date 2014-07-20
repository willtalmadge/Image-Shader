//
//  FSTPrecondition.cpp
//  Image Shader
//
//  Created by William Talmadge on 7/13/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#include "FSTPrecondition.h"

using namespace std;

string FSTPrecondition::fragShader = SHADER_STRING
(
 varying highp vec2 j;
 varying highp vec2 Nmj;
 varying highp vec2 stj;
 
 uniform highp sampler2D tex;
 uniform highp sampler2D phaseTable;
 void main()
{
	highp float st = texture2D(phaseTable, stj).y;
	highp vec4 fj = texture2D(tex, j);
	highp vec4 fNmj = texture2D(tex, Nmj);
	
	highp vec4 result;
	result.rgb = st*(fj.rgb + fNmj.rgb) + 0.5*(fj.rgb - fNmj.rgb);
	result.a = 1.0;
	
	gl_FragColor = result;
}
);

string FSTPrecondition::vertShader = SHADER_STRING
(
 attribute highp vec4 positionIn;
 attribute highp vec2 jIn;
 attribute highp vec2 NmjIn;
 
 uniform highp mat4 orthoMatrix;
 uniform bool isOrientationCol;
 varying highp vec2 j;
 varying highp vec2 Nmj;
 varying highp vec2 stj;
 
 void main()
{
	gl_Position = orthoMatrix*positionIn;
	j = jIn;
	Nmj = NmjIn;
	if (isOrientationCol) {
		stj = jIn;
	} else {
		stj.xy = jIn.yx;
	}
}

);