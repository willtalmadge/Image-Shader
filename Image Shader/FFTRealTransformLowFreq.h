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

#ifndef __Image_Shader__FFTRealTransformLowFreq__
#define __Image_Shader__FFTRealTransformLowFreq__

#include <iostream>
#include <iostream>
#include "ISDrawable.h"
#include "ISComplex.h"
#include "ISSingleton.h"
#include "FFTPhaseTable.h"


struct FFTRealTransformLowFreq: public ISDrawable<ISComplex, ISSingleton, FFTRealTransformLowFreq>, ISComplexBindable {
    
    using ISDrawableT = ISDrawable<ISComplex, ISSingleton, FFTRealTransformLowFreq>;
    
    enum class OutType { Real, Imag };

    FFTRealTransformLowFreq(GLuint width, GLuint height, OutType outType, int direction) : ISDrawableT(width, height), _outType(outType), _orthoMatrixPosition(0) {
        _orthoMatrix = GLKMatrix4MakeOrtho(0.0, _width, 0.0, _height, -1.0, 1.0);
        _sign = -1*direction;
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
        glUniformMatrix4fv(_orthoMatrixPosition, 1, false, _orthoMatrix.m);
        glUniform1f(_signUP, static_cast<GLfloat>(_sign));
        ISTextureRef table = FFTPhaseTable::getPhaseTable(_width, 2*_width, -1*_sign);
        table->bindToShader(_phaseTableUP, inputTuple->textureUnitsUsed());
    }

    size_t hashImpl() const {
        size_t result = 0;
        if (_outType == OutType::Real) {
            result = 0;
        } else if (_outType == OutType::Imag) {
            result = 1;
        }
        return result;
    };
    
    bool compareImpl(const FFTRealTransformLowFreq& rhs) const {
        bool result = true;
        result &= _outType == rhs._outType;
        return result;
    };
    void drawImpl() { };
    void setupGeometry() {
        ISVertexArray* geometry = new ISVertexArray();
        std::vector<GLfloat> vertices;
        makeGlLookupColumnVarying(vertices, _height, 1, _width/2,
                                  {1.0f/_width, 1.0f},
                                  {0.5f, 0.5f + 1.0f/_width});
        geometry->addFloatAttribute(0, 3);
        geometry->addFloatAttribute(1, 2);
        geometry->addFloatAttribute(2, 2);
        geometry->upload(vertices);
        _geometry = geometry;
    }
    void setupShaderProgram(ISShaderProgram* program) {
        std::vector<ShaderAttribute> attributeMap{
            {0, "positionIn"},
            {1, "nIn"},
            {2, "N2mnIn"}
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
    
    static const std::string fragShaderRe;
    static const std::string fragShaderIm;
    
    static const std::string vertShader;
};
#endif /* defined(__Image_Shader__FFTRealTransformLowFreq__) */
