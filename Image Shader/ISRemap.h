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

#ifndef __Image_Shader__ISRemap__
#define __Image_Shader__ISRemap__


#include "ISShaderProgram.h"
#include "ISDrawable.h"
#include "ISVertexArray.h"
#include "ISSingleton.h"
#include "ISTextureIndexing.h"
#include "ISRemappable.h"

struct ISRemap: public ISDrawable<ISRemappable, ISSingleton, ISRemap>, ISRemappableBindable {
    
    
    using ISDrawableT = ISDrawable<ISRemappable, ISSingleton, ISRemap>;
    
    ISRemap() : ISDrawableT() {
    }
    void init() {
    }
    GLuint textureBindingTarget() const {
        assert(_isSetup);
        return _textureUP;
        
    }
    GLuint remapBindingTarget() const {
        assert(_isSetup);
        return _remapUP;
    }
    void bindUniforms(ISRemappable* inputTuple, ISSingleton* outputTuple) {
    }
    size_t hashImpl() const {
        size_t result = 0;
        return result;
    };
    
    bool compareImpl(const ISRemap& rhs) const {
        bool result = true;
        return result;
    };
    void drawImpl() { };
    void setupGeometry() {
        ISVertexArray* geometry = new ISVertexArray();
        ISTextureIndexer xRead, yRead, xWrite, yWrite;
        xRead.reads().from(_sourceROI.left()).upTo(_sourceROI.right()).alongRows(texelWidth());
        yRead.reads().from(_sourceROI.top()).upTo(_sourceROI.bottom()).alongCols(texelHeight());
        xWrite.writes().from(_targetROI.left()).upTo(_targetROI.right()).alongRows(texelWidth());
        yWrite.writes().from(_targetROI.top()).upTo(_targetROI.bottom()).alongCols(texelHeight());
        indexerGeometry2D(geometry, xWrite, yWrite, {xRead}, {yRead});
        _geometry = geometry;
    }
    void setupShaderProgram(ISShaderProgram* program) {
        std::vector<ShaderAttribute> attributeMap{
            {0, "rWrite"},
            {1, "rRead"}
        };
        program->loadShader(fragShader,
                            prependOrthoMatrixUniform(vertShader),
                            attributeMap);
    }
    void resolveUniformPositions() {
        _textureUP = glGetUniformLocation(_program->program(), "texture");
        _remapUP = glGetUniformLocation(_program->program(), "remap");
    }
    
protected:
    static const std::string vertShader;
    static const std::string fragShader;
    
    GLuint _textureUP;
    GLuint _remapUP;
};

#endif /* defined(__Image_Shader__ISRemap__) */
