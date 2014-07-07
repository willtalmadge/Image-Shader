//  Created by William Talmadge on 6/12/14.
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

#ifndef __Image_Shader__FFTPermuteColumns__
#define __Image_Shader__FFTPermuteColumns__

#include <iostream>
#include <vector>
#include "ISShaderProgram.h"
#include "ISDrawable.h"
#include "ISVertexArray.h"
#include "ISSingleton.h"
#include <GLKit/GLKMatrix4.h>
#include <string>


struct FFTPermute : public ISDrawable<ISSingleton, ISSingleton, FFTPermute>, ISSingletonBindable {
    static const std::string fragShader;
    static const std::string vertShaderRows;
    static const std::string vertShaderCols;
    
    using ISDrawableT = ISDrawable<ISSingleton, ISSingleton, FFTPermute>;
    enum class Orientation { Rows, Cols };
    enum class Stride { SkipNone, SkipOne };
    enum class Offset { Zero, One };
    GLfloat stride() {
        if (_stride == Stride::SkipNone) {
            return 1.0;
        } else {
            return 2.0;
        }
    }
    GLfloat offset() {
        if (_offset == Offset::Zero) {
            return 0.0;
        } else  {
            return 1.0;
        }
    }
    GLfloat size() {
        if (_orientation == Orientation::Cols) {
            return _width;
        } else {
            return _height;
        }
    }
    GLfloat length() {
        if (_orientation == Orientation::Rows) {
            return _width;
        } else {
            return _height;
        }
    }
    typedef ISSingleton InputType;
    //TODO: abstract _orthoMatrix to base
    FFTPermute(GLuint width, GLuint height, Stride stride, Offset offset, Orientation orientation, std::vector<GLuint> butterflyPlan) : ISDrawableT(width, height), _orthoMatrixPosition(0), _stride(stride), _offset(offset), _orientation(orientation), _butterflyPlan(butterflyPlan) {
        _orthoMatrix = GLKMatrix4MakeOrtho(0.0, _width, 0.0, _height, -1.0, 1.0);
    }
    GLuint textureBindingTarget() const {
        assert(_isSetup);
        return _textureUniformPosition;
    }
    void bindUniforms(ISSingleton* inputTuple, ISSingleton* outputTuple) {
        if (_orientation == Orientation::Cols) {
            //TODO: this can fire if the source data is a real signal with no factors of 2 in it.
            assert(inputTuple->getTexture()->width() == stride()*outputTuple->getTexture()->width());
        } else if (_orientation == Orientation::Rows) {
            assert(inputTuple->getTexture()->height() == stride()*outputTuple->getTexture()->height());
        }
        glUniformMatrix4fv(_orthoMatrixPosition, 1, false, _orthoMatrix.m);
        glUniform1f(_strideUP, stride());
        glUniform1f(_offsetUP, offset());
        glUniform1f(_targetSizeUP, stride()*size()); 
    }
    size_t hashImpl() const {
        size_t result;
        if (_orientation == Orientation::Cols) {
            result = std::hash<size_t>()(0);
        } else {
            result = std::hash<size_t>()(1);
        }
        for (size_t b : _butterflyPlan) {
            result ^= b;
        }
        return result;
    }
    bool compareImpl(const FFTPermute& rhs) const {
        bool result = true;
        if (_butterflyPlan.size() != rhs._butterflyPlan.size()) {
            result = false;
        } else {
            result = _orientation == rhs._orientation;
            for (size_t i = 0; i < _butterflyPlan.size(); i++) {
                result &= _butterflyPlan[i] == rhs._butterflyPlan[i];
            }
        }
        return result;
    };
    void drawImpl() { };
    void permute_(std::vector<GLuint>& permutation,
                  GLuint sumForward,
                  GLuint sumReverse,
                  GLuint radixForward, //alpha set
                  GLuint radixReverse, //beta set
                  std::vector<GLuint>::iterator radixFactor,
                  std::vector<GLuint>::iterator end)
    {
        
        if (radixFactor != end)
        {
            std::vector<GLuint>::iterator nextRadixFactor = radixFactor;
            ++nextRadixFactor;
            for (GLuint s = 0; s < *radixFactor; s++)
            {
                
                permute_(permutation,
                         sumForward + s*radixForward,
                         sumReverse + s*(radixReverse/(*radixFactor)),
                         radixForward*(*radixFactor),
                         radixReverse/(*radixFactor),
                         nextRadixFactor,
                         end);
            }
        }
        else
        {
            permutation[sumReverse] = sumForward;
        }
    }
    void setupGeometry() {
        //TODO: detect orientation
        ISVertexArray* geometry = new ISVertexArray();
        //First reverse the butterfly set so we start from R_0 instead of R_M-1
        reverse(_butterflyPlan.begin(), _butterflyPlan.end());
        int n = 1;
        for (GLuint i = 0; i < _butterflyPlan.size();i++) {
            n *= _butterflyPlan[i];
        }
        assert(n == size());
        std::vector<GLuint> permutationTable(size());
        permutationTable.resize(size());
        permute_(permutationTable, 0, 0, 1, size(), _butterflyPlan.begin(), _butterflyPlan.end());
        
        std::vector<GLfloat> vertices;
        vertices.reserve(size()*5*6);
        if (_orientation == Orientation::Cols) {
            for (GLuint i = 0; i < size(); i++) {
                GLfloat source = static_cast<GLfloat>(permutationTable[i]);
                std::vector<GLfloat>
                quad = makeGlAttributePixelColumn(length(), i, i+1,
                                                  {0.0, source}, {1.0, source}); //This is a bit confusing but the vert shader actually may swaps these depending on the orientation
                vertices.insert(vertices.end(), quad.begin(), quad.end());
            }
        } else {
            for (GLuint i = 0; i < size(); i++) {
                GLfloat source = static_cast<GLfloat>(permutationTable[i]);
                appendGlLookupRow(vertices, length(), i, i+1, {source}, {source}, {}, {});
            }
        }
        geometry->addFloatAttribute(0, 3);
        geometry->addFloatAttribute(1, 1);
        geometry->addFloatAttribute(2, 1);
        geometry->upload(vertices);
        _geometry = geometry;
    }
    void setupShaderProgram(ISShaderProgram* program) {
        std::vector<ShaderAttribute> attributeMap{
            {0, "positionIn"},
            {1, "texCoordVIn"},
            {2, "sourceIndexIn"}
        };
        if (_orientation == Orientation::Cols) {
            program->loadShader(fragShader, vertShaderCols, attributeMap);
        } else {
            program->loadShader(fragShader, vertShaderRows, attributeMap);
        }
    }
    void resolveUniformPositions() {
        _orthoMatrixPosition = glGetUniformLocation(_program->program(), "orthoMatrix");
        _textureUniformPosition = glGetUniformLocation(_program->program(), "inputImageTexture");
        _strideUP = glGetUniformLocation(_program->program(), "stride");
        _offsetUP = glGetUniformLocation(_program->program(), "offset");
        _targetSizeUP = glGetUniformLocation(_program->program(), "targetSize");
    }
    
protected:
    std::vector<GLuint> _butterflyPlan;
    
    GLuint _textureUniformPosition;
    GLuint _orthoMatrixPosition;
    GLuint _strideUP;
    GLuint _offsetUP;
    GLuint _targetSizeUP;
    
    GLKMatrix4 _orthoMatrix;
    Orientation _orientation;
    Stride _stride;
    Offset _offset;
};
#endif /* defined(__Image_Shader__FFTPermuteColumns__) */
