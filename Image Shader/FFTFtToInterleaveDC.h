//
//  FFTFtToInterleaveDC.h
//  Image Shader
//
//  Created by William Talmadge on 7/2/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#ifndef __Image_Shader__FFTFtToInterleaveDC__
#define __Image_Shader__FFTFtToInterleaveDC__

#include <iostream>
#include "ISDrawable.h"
#include "ISComplex.h"
#include "ISSingleton.h"

struct FFTFtToInterleaveDC : public ISDrawable<ISComplex, ISSingleton, FFTFtToInterleaveDC>, ISComplexBindable
{
    using ISDrawableT = ISDrawable<ISComplex, ISSingleton, FFTFtToInterleaveDC>;
    typedef ISComplex InputType; //FIXME: when it is building, try to exclude this
    
    enum class OutType { Real, Imag };
    //TODO: take arguments to the drawable in the constructor
    FFTFtToInterleaveDC(GLuint width, GLuint height, OutType outType) : ISDrawableT(width, height), _outType(outType), _orthoMatrixPosition(0) {
        _orthoMatrix = GLKMatrix4MakeOrtho(0.0, _width, 0.0, _height, -1.0, 1.0);
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
        //TODO: set uniforms
        glUniformMatrix4fv(_orthoMatrixPosition, 1, false, _orthoMatrix.m);
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
    bool compareImpl(const FFTFtToInterleaveDC& rhs) const {
        bool result = true;
        result &= _outType == rhs._outType;
        return result;
    };
    void drawImpl() { };
    void setupGeometry() {
        ISVertexArray* geometry = new ISVertexArray();
        std::vector<GLfloat> vertices
        (
         makeGlAttributePixelColumn(_height, 0, 1,
                                    {static_cast<GLfloat>(0.5/_width), 0.0},
                                    {static_cast<GLfloat>(0.5/_width), 1.0})
         );
        
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
            program->loadShader(fragShaderRe, vertShader, attributeMap);
        } else if (_outType == OutType::Imag) {
            program->loadShader(fragShaderIm, vertShader, attributeMap);
        }
    }
    void resolveUniformPositions() {
        _orthoMatrixPosition = glGetUniformLocation(_program->program(), "orthoMatrix");
        _inputReUP = glGetUniformLocation(_program->program(), "inputReUP");
        _inputImUP = glGetUniformLocation(_program->program(), "inputImUP");
    }
    
protected:
    
    GLuint _orthoMatrixPosition;
    GLuint _inputReUP;
    GLuint _inputImUP;
    //TODO: define uniform positions
    
    GLKMatrix4 _orthoMatrix;
    OutType _outType;
    //TODO: define argument storage
    static const std::string fragShaderRe;
    static const std::string fragShaderIm;
    static const std::string vertShader;
};

#endif /* defined(__Image_Shader__FFTFtToInterleaveDC__) */
