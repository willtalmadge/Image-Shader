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

#ifndef __Image_Shader__FFTRealTransformNDC__
#define __Image_Shader__FFTRealTransformNDC__

#include <iostream>
#include "ISDrawable.h"
#include "ISComplex.h"
#include "ISSingleton.h"
#include "ISTextureIndexing.h"

//A drawable for converting an interleaved transform (treating evens as reals and
//odds as imaginary in the FT) to an FT. Draws only to the first pixel column
//where the DC and Nyquist components are stored.
//Real shader computes the DC component
//Imaginary shader computes the Nyquist component (real) and packs it into the imaginary component
//This shader exists for one reason, the Nyquest component needs to be written into the imgainary
//part of the DC component to avoid allocating a whole texture for 1 column of pixels.

struct FFTRealTransformNDC : public ISDrawable<ISComplex, ISSingleton, FFTRealTransformNDC>, ISComplexBindable
{
    using ISDrawableT = ISDrawable<ISComplex, ISSingleton, FFTRealTransformNDC>;
    
    enum class OutType { Real, Imag };
    FFTRealTransformNDC(OutType outType, int sign, ISDirection direction) : ISDrawableT(), _outType(outType), _direction(direction) {
        if (sign == 1) {
            _multiplier = 1.0;
        } else if (sign == -1) {
            _multiplier = 0.5;
        }
    }
    void init() { }
    GLuint realBindingTarget() const {
        assert(_isSetup);
        return _inputReUP;
    }
    GLuint imagBindingTarget() const {
        assert(_isSetup);
        return _inputImUP;
    }
    void bindUniforms(ISComplex* inputTuple, ISSingleton* outputTuple) {
        glUniform1f(_multiplierUP, _multiplier);
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
        } else if (_direction == ISDirection::Cols) {
            result ^= 1;
        }
        return result;
    };
    bool compareImpl(const FFTRealTransformNDC& rhs) const {
        bool result = true;
        result &= _outType == rhs._outType;
        result &= _direction == rhs._direction;
        return result;
    };
    void drawImpl() { };
    void setupGeometry() {
        ISVertexArray* geometry = new ISVertexArray();
        std::vector<GLfloat> vertices;
        uint width = _targetROI.width();
        uint height = _targetROI.height();
        if (_direction == ISDirection::Rows) {
            vertices = makeGlAttributePixelColumn(height, 0, 1,
                                                  {static_cast<GLfloat>(0.5/width), 0.0f},
                                                  {static_cast<GLfloat>(0.5/width), 1.0f});
        } else if (_direction == ISDirection::Cols) {
            vertices = makeGlAttributePixelRow(width, 0, 1,
                                               {0.0f, static_cast<GLfloat>(0.5/height)},
                                               {1.0, static_cast<GLfloat>(0.5/height)});
        }
        geometry->addFloatAttribute(0, 3);
        geometry->addFloatAttribute(1, 2);
        
        geometry->upload(vertices);
        _geometry = geometry;
    }
    void setupShaderProgram(ISShaderProgram* program) {
        std::vector<ShaderAttribute> attributeMap{
            {0, "positionIn"},
            {1, "nIn"}
        };
        if (_outType == OutType::Real) {
            program->loadShader(fragShaderRe, prependOrthoMatrixUniform(vertShader), attributeMap);
        } else if (_outType == OutType::Imag) {
            program->loadShader(fragShaderIm, prependOrthoMatrixUniform(vertShader), attributeMap);
        }
    }
    void resolveUniformPositions() {
        _inputReUP = glGetUniformLocation(_program->program(), "inputRe");
        _inputImUP = glGetUniformLocation(_program->program(), "inputIm");
        _multiplierUP = glGetUniformLocation(_program->program(), "c");
    }
    
protected:
    
    GLuint _inputReUP;
    GLuint _inputImUP;
    GLuint _multiplierUP;
    
    OutType _outType;
    GLfloat _multiplier;
    ISDirection _direction;
    
    static const std::string fragShaderRe;
    static const std::string fragShaderIm;
    static const std::string vertShader;
};

#endif /* defined(__Image_Shader__FFTRealTransformNDC__) */
