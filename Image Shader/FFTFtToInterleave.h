//
//  FFTFtToInterleave.h
//  Image Shader
//
//  Created by William Talmadge on 7/1/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#ifndef __Image_Shader__FFTFtToInterleave__
#define __Image_Shader__FFTFtToInterleave__

#include <iostream>
#include "ISDrawable.h"
#include "ISComplex.h"
#include "ISSingleton.h"
#include "FFTPhaseTable.h"

//An NDC version of this shader isn't needed because the output storage is
//as expected for the equation. Only the input of the first column needs its N2mn
//values set to be the same as n


//Write the shaders first. Make the drawable support your shader.
//Define the type of your drawable
struct FFTFtToInterleave: public ISDrawable<ISComplex, ISSingleton, FFTFtToInterleave>, ISComplexBindable {
    
    using ISDrawableT = ISDrawable<ISComplex, ISSingleton, FFTFtToInterleave>;
    typedef ISComplex InputType; //FIXME: when it is building, try to exclude this
    
    enum class OutType { Real, Imag };
    FFTFtToInterleave(GLuint width, GLuint height, OutType outType) : ISDrawableT(width, height), _outType(outType), _orthoMatrixPosition(0) {
        _orthoMatrix = GLKMatrix4MakeOrtho(0.0, _width, 0.0, _height, -1.0, 1.0);
    }
    //Implement the methods for the ISBindableType interface i.e. ISSingletonBindable as below
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
        ISTextureRef table = FFTPhaseTable::getPhaseTable(_width, -1);
        table->bindToShader(_phaseTableUP, inputTuple->textureUnitsUsed());
    }
    //TODO: hash parameters that determine geometry or shader source (not uniforms)
    size_t hashImpl() const {
        size_t result = 0;
        return result;
        
    };
    //TODO: compare parameters that determine geometry or shader source (not uniforms)
    
    bool compareImpl(const FFTFtToInterleave& rhs) const {
        bool result = true;
        return result;
    };
    void drawImpl() { };
    void setupGeometry() {
        ISVertexArray* geometry = new ISVertexArray();
        std::vector<GLfloat> vertices;
        //TODO: construct geometry
        //makeGlLookupColumnVarying(vertices, _height, 0, 1, {0.0f, 0.0f}, {1.0f/_width, 1.0f/_width});
        makeGlLookupColumnVarying(vertices, _height, 1, _width,
                                  {1.0f/_width, 1.0},
                                  {1.0f, 1.0f/_width});
        //TODO: set attributes
        geometry->addFloatAttribute(0, 3);
        geometry->addFloatAttribute(1, 2);
        geometry->addFloatAttribute(2, 2);
        //TODO: upload vertices
        geometry->upload(vertices);
        _geometry = geometry;
    }
    void setupShaderProgram(ISShaderProgram* program) {
        //TODO: create shader source string and load shader, use shader cache if possible
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
        //TODO: verify attribute order match those in setupGeometry()
    }
    void resolveUniformPositions() {
        _orthoMatrixPosition = glGetUniformLocation(_program->program(), "orthoMatrix");
        _inputReUP = glGetUniformLocation(_program->program(), "inputRe");
        _inputImUP = glGetUniformLocation(_program->program(), "inputIm");
        _phaseTableUP = glGetUniformLocation(_program->program(), "phaseTable");
    }
    
protected:
    
    
    GLuint _inputReUP;
    GLuint _inputImUP;
    GLuint _phaseTableUP;
    GLuint _orthoMatrixPosition;
    //TODO: define uniform positions
    
    GLKMatrix4 _orthoMatrix;
    OutType _outType;
    //TODO: define argument storage
    
    static const std::string fragShaderRe;
    static const std::string fragShaderIm;
    
    static const std::string vertShader;
};

#endif /* defined(__Image_Shader__FFTFtToInterleave__) */
