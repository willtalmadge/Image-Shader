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

#ifndef __Image_Shader__FSTPrecondition__
#define __Image_Shader__FSTPrecondition__

#include <string>
#include <vector>
#include "ISDrawable.h"
#include "ISSingleton.h"
#include "FFTPhaseTable.h"
#include "ISTextureIndexing.h"

//The values for the first element of the signal must be zero. This shader accomplishes that by ensuring the glClearColor is zeros except for the alpha and simply not drawing anything in the first row/column
struct FSTPrecondition: public ISDrawable<ISSingleton, ISSingleton, FSTPrecondition>, ISSingletonBindable {
    
    using ISDrawableT = ISDrawable<ISSingleton, ISSingleton, FSTPrecondition>;

    enum class Orientation { Rows, Cols }; //Cols orientation indicates to perform the transform across the columns
    
    FSTPrecondition(Orientation orientation, GLfloat scale=1.0f) : ISDrawableT(), _textureUP(0), _phaseTableUP(0), _orientation(orientation), _scale(scale) {
    }
    void init() {
        assert(_sourceROI.size() == _targetROI.size());
    }
    GLuint textureBindingTarget() const {
        assert(_isSetup);
        return _textureUP;
    }
    void bindUniforms(ISSingleton* inputTuple, ISSingleton* outputTuple) {
        uint width = _targetROI.width();
        uint height = _targetROI.height();
        ISTextureRef phaseTable;
        if (_orientation == Orientation::Cols) {
            phaseTable = FFTPhaseTable::getPhaseTable(width, 2*width, 1);
        } else if (_orientation == Orientation::Rows) {
            phaseTable = FFTPhaseTable::getPhaseTable(height, 2*height, 1);
        }
        phaseTable->bindToShader(_phaseTableUP, inputTuple->textureUnitsUsed());
        glUniform1f(_scaleUP, _scale);
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
    void drawImpl() {
    //TODO: clear the first pixel column
    };
    void setupGeometry() {
        uint width = _targetROI.width();
        uint height = _targetROI.height();
        ISVertexArray* geometry = new ISVertexArray();

        ISTextureIndexer jxWrite, jyWrite, jxRead, jyRead, NmjxRead, NmjyRead, stjxRead, stjyRead;
        if (_orientation == Orientation::Cols) {
            jxWrite.writes().from(1).upTo(width).alongRows(width);
            jyWrite.writes().from(0).upTo(height).alongCols(height);
            jxRead.reads().from(1).upTo(width).alongRows(_sourceSize.width());
            jyRead.reads().from(0).upTo(height).alongCols(_sourceSize.height());
            NmjxRead.reads().from(1).upTo(width).by(-1).offset(width).alongRows(_sourceSize.width());
            NmjyRead.reads().from(0).upTo(height).alongCols(_sourceSize.height());
            stjxRead.reads().from(1).upTo(width).alongRows(width);
            stjyRead.reads().from(0).upTo(1).alongCols(1); //No ROI for phase tables
            
        } else if (_orientation == Orientation::Rows) {
            jxWrite.writes().from(0).upTo(width).alongRows(width);
            jyWrite.writes().from(1).upTo(height).alongCols(height);
            jxRead.reads().from(0).upTo(width).alongRows(_sourceSize.width());
            jyRead.reads().from(1).upTo(height).alongCols(_sourceSize.height());
            NmjxRead.reads().from(0).upTo(width).alongRows(_sourceSize.width());
            NmjyRead.reads().from(1).upTo(height).by(-1).offset(height).alongCols(_sourceSize.height());
            stjxRead.reads().from(0).upTo(1).alongCols(1);
            stjyRead.reads().from(1).upTo(height).alongRows(height);
        }
        indexerGeometry2D(geometry, jxWrite, jyWrite, {
            jxRead,
            NmjxRead,
            stjxRead
        }, {
            jyRead,
            NmjyRead,
            stjyRead
        });

        _geometry = geometry;
    }
    void setupShaderProgram(ISShaderProgram* program) {
        std::vector<ShaderAttribute> attributeMap{
            {0, "jWrite"},
            {1, "jRead"},
            {2, "NmjRead"},
            {3, "stjRead"}
        };
        program->loadShader(fragShader, prependOrthoMatrixUniform(vertShader), attributeMap);
        //TODO: create shader source string and load shader, use shader cache if possible
    }
    void resolveUniformPositions() {
        _textureUP = glGetUniformLocation(_program->program(), "tex");
        _phaseTableUP = glGetUniformLocation(_program->program(), "phaseTable");
        _scaleUP = glGetUniformLocation(_program->program(), "scale");
    }
    
protected:
    static std::string fragShader;
    static std::string vertShader;
    
    GLuint _textureUP;
    GLuint _phaseTableUP;
    GLuint _isOrientationColUP;
    GLuint _scaleUP;
    
    GLfloat _scale;
    Orientation _orientation;
};
#endif /* defined(__Image_Shader__FSTPrecondition__) */
