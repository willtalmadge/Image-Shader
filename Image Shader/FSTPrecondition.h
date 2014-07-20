//
//  FSTPrecondition.h
//  Image Shader
//
//  Created by William Talmadge on 7/13/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#ifndef __Image_Shader__FSTPrecondition__
#define __Image_Shader__FSTPrecondition__

#include <string>
#include <vector>
#include "ISDrawable.h"
#include "ISSingleton.h"
#include "FFTPhaseTable.h"

//The values for the first element of the signal must be zero. This shader accomplishes that by ensuring the glClearColor is zeros except for the alpha and simply not drawing anything in the first row/column
struct FSTPrecondition: public ISDrawable<ISSingleton, ISSingleton, FSTPrecondition>, ISSingletonBindable {
    
    using ISDrawableT = ISDrawable<ISSingleton, ISSingleton, FSTPrecondition>;

    enum class Orientation { Rows, Cols }; //Cols orientation indicates to perform the transform across the columns
    
    FSTPrecondition(GLuint width, GLuint height, Orientation orientation) : ISDrawableT(width, height), _orthoMatrixUP(0), _textureUP(0), _phaseTableUP(0), _orientation(orientation) {
        _orthoMatrix = GLKMatrix4MakeOrtho(0.0, _width, 0.0, _height, -1.0, 1.0);
    }
    GLuint textureBindingTarget() const {
        assert(_isSetup);
        return _textureUP;
    }
    void bindUniforms(ISSingleton* inputTuple, ISSingleton* outputTuple) {
        glUniformMatrix4fv(_orthoMatrixUP, 1, false, _orthoMatrix.m);
        ISTextureRef phaseTable;
        if (_orientation == Orientation::Cols) {
            phaseTable = FFTPhaseTable::getPhaseTable(_width, 2*_width, 1);
            glUniform1i(_isOrientationColUP, 1);
        } else if (_orientation == Orientation::Rows) {
            phaseTable = FFTPhaseTable::getPhaseTable(_height, 2*_height, 1);
            glUniform1i(_isOrientationColUP, 0);
        }
        phaseTable->bindToShader(_phaseTableUP, inputTuple->textureUnitsUsed());
    }

    size_t hashImpl() const {
        size_t result = 0;
        if (_orientation == Orientation::Cols) {
            result ^= std::hash<size_t>()(0);
        } else {
            result ^= std::hash<size_t>()(1);
        }
        return result;
    };
    bool compareImpl(const FSTPrecondition& rhs) const {
        bool result = true;
        result &= (_orientation == rhs._orientation);
        return result;
    };
    void drawImpl() { };
    void setupGeometry() {
        ISVertexArray* geometry = new ISVertexArray();
        std::vector<GLfloat> vertices;
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        if (_orientation == Orientation::Cols) {
            makeGlLookupColumnVarying(vertices, _height, 1, _width,
                                      {1.0f/_width, 1.0f},
                                      {1.0f, 1.0f/_width});
        } else if (_orientation == Orientation::Rows) {
            appendGlLookupRow(vertices, _width, 1, _height,
                              {1.0f/_width, 1.0f},
                              {1.0f, 1.0f/_width}, {}, {});
        }
        geometry->addFloatAttribute(0, 3);
        geometry->addFloatAttribute(1, 2);
        geometry->addFloatAttribute(2, 2);
        geometry->upload(vertices);
        _geometry = geometry;
    }
    void setupShaderProgram(ISShaderProgram* program) {
        std::vector<ShaderAttribute> attributeMap{
            {0, "positionIn"},
            {1, "jIn"},
            {2, "NmjIn"}
        };
        program->loadShader(fragShader, vertShader, attributeMap);
        //TODO: create shader source string and load shader, use shader cache if possible
    }
    void resolveUniformPositions() {
        _orthoMatrixUP = glGetUniformLocation(_program->program(), "orthoMatrix");
        _textureUP = glGetUniformLocation(_program->program(), "tex");
        _phaseTableUP = glGetUniformLocation(_program->program(), "phaseTable");
        _isOrientationColUP = glGetUniformLocation(_program->program(), "isOrientationCol");
    }
    
protected:
    static std::string fragShader;
    static std::string vertShader;
    
    GLuint _textureUP;
    GLuint _orthoMatrixUP;
    GLuint _phaseTableUP;
    GLuint _isOrientationColUP;
    
    GLKMatrix4 _orthoMatrix;
    Orientation _orientation;
};
#endif /* defined(__Image_Shader__FSTPrecondition__) */
