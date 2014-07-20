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

#ifndef __Image_Shader__FFTRealTransformHighFreq__
#define __Image_Shader__FFTRealTransformHighFreq__

#include <iostream>
#include <iostream>
#include "ISDrawable.h"
#include "ISComplex.h"
#include "ISSingleton.h"
#include "FFTPhaseTable.h"
#include "ISTextureIndexing.h"

struct FFTRealTransformHighFreq: public ISDrawable<ISComplex, ISSingleton, FFTRealTransformHighFreq>, ISComplexBindable {
    
    using ISDrawableT = ISDrawable<ISComplex, ISSingleton, FFTRealTransformHighFreq>;
    
    enum class OutType { Real, Imag };
    FFTRealTransformHighFreq(GLuint width, GLuint height, OutType outType, int sign, ISDirection direction) : ISDrawableT(width, height), _outType(outType), _orthoMatrixPosition(0), _direction(direction) {
        _orthoMatrix = GLKMatrix4MakeOrtho(0.0, _width, 0.0, _height, -1.0, 1.0);
        _sign = -1*sign;
    }
    GLuint realBindingTarget() const {
        assert(_isSetup);
        return _inputReUP;
    }
    GLuint imagBindingTarget() const {
        assert(_isSetup);
        return _inputImUP;
    }
    void bindUniforms(ISComplex* inputTuple, ISSingleton* outputTuple) {
        //TODO: set uniforms (not samplers, that is automatic)
        glUniformMatrix4fv(_orthoMatrixPosition, 1, false, _orthoMatrix.m);
        glUniform1f(_signUP, static_cast<GLfloat>(_sign));
        ISTextureRef table = NULL;
        if (_direction == ISDirection::Rows) {
            table = FFTPhaseTable::getPhaseTable(_width, 2*_width, -1*_sign);
        } else {
            table = FFTPhaseTable::getPhaseTable(_height, 2*_height, -1*_sign);
        }
        table->bindToShader(_phaseTableUP, inputTuple->textureUnitsUsed());
    }
    size_t hashImpl() const {
        size_t result = 0;
        if (_outType == OutType::Real) {
            result = 0;
        } else if (_outType == OutType::Imag) {
            result = 1;
        }
        if (_direction == ISDirection::Rows) {
            result ^= 0;
        } else {
            result ^= 1;
        }
        return result;
    };
    
    bool compareImpl(const FFTRealTransformHighFreq& rhs) const {
        bool result = true;
        result &= _outType == rhs._outType;
        result &= _direction == rhs._direction;
        return result;
    };
    void drawImpl() { };
    void setupGeometry() {
        ISVertexArray* geometry = new ISVertexArray();

        ISTextureIndexer nWrite, nRead, N2mnRead, ptnRead;
        
        if (_direction == ISDirection::Rows) {
            nWrite.writes().from(1).to(_width/2 + 1).by(-1).offset(_width).alongRows(_width);
            nRead.reads().from(1).to(_width/2 + 1).alongRows(_width);
            N2mnRead.reads().from(1).to(_width/2 + 1).by(-1).offset(_width).alongRows(_width);
            ptnRead.reads().from(1).to(_width/2 + 1).alongRows(_width);
        } else {
            nWrite.writes().from(1).to(_height/2 + 1).by(-1).offset(_height).alongCols(_height);
            nRead.reads().from(1).to(_height/2 + 1).alongCols(_height);
            N2mnRead.reads().from(1).to(_height/2 + 1).by(-1).offset(_height).alongCols(_height);
            ptnRead.reads().from(1).to(_height/2 + 1).alongRows(_height); //Phase table always a row
        }
        
        indexerGeometry(geometry, _width, _height, nWrite, {
            nRead,
            N2mnRead,
            ptnRead
        });
        
        _geometry = geometry;
    }
    void setupShaderProgram(ISShaderProgram* program) {
        std::vector<ShaderAttribute> attributeMap{
            {0, "nWrite"},
            {1, "nRead"},
            {2, "N2mnRead"},
            {3, "ptnRead"}
        };
        if (_outType == OutType::Real) {
            program->loadShader(fragShaderRe, vertShader, attributeMap);
        } else if (_outType == OutType::Imag) {
            program->loadShader(fragShaderIm, vertShader, attributeMap);
        }
    }
    void resolveUniformPositions() {
        _orthoMatrixPosition = glGetUniformLocation(_program->program(), "orthoMatrix");
        _inputReUP = glGetUniformLocation(_program->program(), "inputRe");
        _inputImUP = glGetUniformLocation(_program->program(), "inputIm");
        _phaseTableUP = glGetUniformLocation(_program->program(), "phaseTable");
        _signUP = glGetUniformLocation(_program->program(), "s");
    }
    
protected:
    
    GLuint _inputReUP;
    GLuint _inputImUP;
    GLuint _phaseTableUP;
    GLuint _orthoMatrixPosition;
    GLuint _signUP;
    
    GLKMatrix4 _orthoMatrix;
    OutType _outType;
    GLint _sign;
    ISDirection _direction;
    
    static const std::string fragShaderRe;
    static const std::string fragShaderIm;
    
    static const std::string vertShader;
};
#endif /* defined(__Image_Shader__FFTRealTransformHighFreq__) */
