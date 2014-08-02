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

#ifndef __Image_Shader__ISPassThroughDrawable__
#define __Image_Shader__ISPassThroughDrawable__

#include <iostream>
#include "ISShaderProgram.h"
#include "ISDrawable.h"
#include "ISVertexArray.h"
#include "ISSingleton.h"
#include "ISTextureIndexing.h"
#include <GLKit/GLKMatrix4.h>
#include <string>
#include "assert.h"

static const std::string fragShader = SHADER_STRING
(
 varying highp vec2 textureCoordinate;
 uniform sampler2D inputImageTexture;
 uniform highp float colorMultiplier;
 
 void main()
{
    highp vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);
    textureColor.rgb = colorMultiplier*textureColor.rgb;
    gl_FragColor = textureColor;
}
 );

static const std::string vertShader = SHADER_STRING
(
 attribute vec4 positionIn;
 attribute vec4 texCoordIn;
 
 varying vec2 textureCoordinate;
 
 void main()
{
    gl_Position = orthoMatrix*positionIn;
    textureCoordinate = texCoordIn.xy;
}
 
 );

//TODO: is output type always an ISSingleton? Maybe omit that?
//TODO: how can I make drawables elegantly take single textures instead of singletons? Or streamline it.
struct ISPassThroughDrawable : public ISDrawable<ISSingleton, ISSingleton, ISPassThroughDrawable>, ISSingletonBindable {
    typedef ISSingleton InputType;
    using ISDrawableT = ISDrawable<ISSingleton, ISSingleton, ISPassThroughDrawable>;
    //TODO: orthomatrix uses drawable dimensions, viewport uses texture dimensions, resolve, which is appropriate?
    ISPassThroughDrawable(GLfloat colorMultiplier = 1.0) : ISDrawableT(), _colorMultiplier(colorMultiplier) { }
    void init() { }
    GLuint textureBindingTarget() const {
        assert(_isSetup);
        return _textureUniformPosition;
    }
    void bindUniforms(ISSingleton* inputTuple, ISSingleton* outputTuple) {
        glUniform1f(_colorMultiplierPosition, _colorMultiplier);
    }
    size_t hashImpl() const { return 0; }
    bool compareImpl(const ISPassThroughDrawable& rhs) const { return true; };
    void drawImpl() { };
    void setupGeometry() {
        ISVertexArray* geometry = new ISVertexArray;
        ISTextureIndexer xWrite, yWrite, xRead, yRead;
        xWrite.writes().from(_targetROI.left()).upTo(_targetROI.right()).alongRows(_targetROI.width());
        yWrite.writes().from(_targetROI.top()).upTo(_targetROI.bottom()).alongCols(_targetROI.height());
        xRead.reads().from(_sourceROI.left()).upTo(_sourceROI.right()).alongRows(_sourceROI.width());
        yRead.reads().from(_sourceROI.top()).upTo(_sourceROI.bottom()).alongCols(_sourceROI.height());
        indexerGeometry2D(geometry, xWrite, yWrite, {xRead}, {yRead});
        _geometry = geometry;
    }
    void setupShaderProgram(ISShaderProgram* program) {
        program->loadShader(fragShader, prependOrthoMatrixUniform(vertShader), {
            {0, "positionIn"},
            {1, "texCoordIn"}
        });
    }
    void resolveUniformPositions() {
        _colorMultiplierPosition = glGetUniformLocation(_program->program(), "colorMultiplier");
        _textureUniformPosition = glGetUniformLocation(_program->program(), "inputImageTexture");
    }
    
    friend class std::hash<ISPassThroughDrawable>;
protected:

    GLuint _textureUniformPosition;
    GLuint _colorMultiplierPosition;
    GLfloat _colorMultiplier;
};

#endif /* defined(__Image_Shader__ISPassThroughDrawable__) */
