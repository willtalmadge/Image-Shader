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

#ifndef __Image_Shader__FFTGaussianFilter__
#define __Image_Shader__FFTGaussianFilter__

#include <iostream>
#include "ISDrawable.h"
#include "ISSingleton.h"

struct FFTGaussianFilter : public ISDrawable<ISSingleton, ISSingleton, FFTGaussianFilter>, ISSingletonBindable {
    
    
    using ISDrawableT = ISDrawable<ISSingleton, ISSingleton, FFTGaussianFilter>;
    
    FFTGaussianFilter(GLuint width, GLuint height, GLfloat sigma) : ISDrawableT(width, height), _sigma(sigma),_orthoMatrixPosition(0) {
        _orthoMatrix = GLKMatrix4MakeOrtho(0.0, _width, 0.0, _height, -1.0, 1.0);
    }
    GLuint textureBindingTarget() const {
        assert(_isSetup);
        return _texUP;
    }
    void bindUniforms(ISSingleton* inputTuple, ISSingleton* outputTuple) {
        glUniformMatrix4fv(_orthoMatrixPosition, 1, false, _orthoMatrix.m);
        glUniform1f(_wSqUP, 4.0*_width*_width); //Assuming operating on a complex type
        glUniform1f(_hSqUP, _height*_height);
        glUniform1f(_varUP, _sigma*_sigma);
    }
    size_t hashImpl() const {
        size_t result = 0;
        result ^= std::hash<float>()(_sigma);
        return result;
        
    };
    
    bool compareImpl(const FFTGaussianFilter& rhs) const {
        bool result = true;
        result &= _sigma == rhs._sigma;
        return result;
    };
    void drawImpl() { };
    void setupGeometry() {
        ISVertexArray* geometry = new ISVertexArray();
        std::vector<GLfloat> vertices;
        GLfloat w = _width/2.0;
        GLfloat h = _height/2.0;
        GLfloat gQuadData[84] =
        {
            // Data layout for each line below is:
            // x, y, z,  iu, iv, rx, ry
            
            //Positive frequency filter
            0.0,    0.0,   0.5f,  0.0, 0.0, 0.0, 0.0,
            w,      0.0,   0.5f,  0.5, 0.0, w, 0.0,
            0.0,    h,     0.5f,  0.0, 0.5, 0.0, h,
            
            w,      h,     0.5f,  0.5, 0.5, w, h,
            0.0,    h,     0.5f,  0.0, 0.5, 0.0, h,
            w,      0.0,   0.5f,  0.5, 0.0, w, 0.0,
            //Negative frequency filter
            0.0,    h,     0.5f,  0.0, 0.5, 0.0, h,
            w,      h,     0.5f,  0.5, 0.5, w, h,
            0.0,    2*h,   0.5f,  0.0, 1.0, 0.0, 0.0,
            
            w,      2*h,   0.5f,  0.5, 1.0, w, 0.0,
            0.0,    2*h,   0.5f,  0.0, 1.0, 0.0, 0.0,
            w,      h,     0.5f,  0.5, 0.5, w, h

        };
        geometry->addFloatAttribute(0, 3);
        geometry->addFloatAttribute(1, 2);
        geometry->addFloatAttribute(2, 2);

        geometry->upload((uchar*)gQuadData, sizeof(gQuadData));
        _geometry = geometry;
    }
    void setupShaderProgram(ISShaderProgram* program) {
        std::vector<ShaderAttribute> attributeMap{
            {0, "positionIn"},
            {1, "iIn"},
            {2, "rIn"}
        };
        program->loadShader(fragShader, vertShader, attributeMap);
    }
    void resolveUniformPositions() {
        _orthoMatrixPosition = glGetUniformLocation(_program->program(), "orthoMatrix");
        _texUP = glGetUniformLocation(_program->program(), "tex");
        _wSqUP = glGetUniformLocation(_program->program(), "wSq");
        _hSqUP = glGetUniformLocation(_program->program(), "hSq");
        _varUP = glGetUniformLocation(_program->program(), "var");
    }
    
protected:
    static const std::string fragShader;
    static const std::string vertShader;
    
    GLuint _texUP;
    GLuint _orthoMatrixPosition;
    GLuint _wSqUP;
    GLuint _hSqUP;
    GLuint _varUP;
    
    GLKMatrix4 _orthoMatrix;
    GLfloat _sigma;
    
};
#endif /* defined(__Image_Shader__FFTGaussianFilter__) */
