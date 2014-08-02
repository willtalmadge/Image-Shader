//  Created by William Talmadge on 6/18/14.
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

#ifndef __Image_Shader__FFTSubBlock__
#define __Image_Shader__FFTSubBlock__

#include <iostream>

#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include "ISDrawable.h"
#include "ISSingleton.h"
#include "ISComplex.h"

struct FFTSubBlock : public ISDrawable<ISComplex, ISSingleton, FFTSubBlock>, ISComplexBindable {

    using ISDrawableT = ISDrawable<ISComplex, ISSingleton, FFTSubBlock>;
#pragma mark - Class sub types
    enum class OutType { Real, Imag };
    enum class Orientation { Rows, Cols };
    GLuint outType() const {
        if (_outType == OutType::Real) {
            return 0;
        } else {
            return 1;
        }
    }
    GLfloat size() {
        if (_orientation == Orientation::Cols) {
            return _targetROI.width();
        } else {
            return _targetROI.height();
        }
    }
    GLfloat length() {
        if (_orientation == Orientation::Rows) {
            return _targetROI.width();
        } else {
            return _targetROI.height();
        }
    }
    struct TexelSamplerRange {
        GLfloat start;
        GLfloat finish;
    };
    static TexelSamplerRange windowIndexer(GLuint indexerStride, GLuint w1, GLuint w2) {
        return TexelSamplerRange{indexerStride*(w1 - 0.5f) + 0.5f, indexerStride*(w2 - 0.5f) + 0.5f};
    }
    struct SubBlock {
        SubBlock(GLuint butterflySize, GLfloat aStart, GLfloat length) : sources(std::vector<TexelSamplerRange>(butterflySize)), start(aStart), finish(aStart + length) { };
        GLfloat start;
        GLfloat finish;
        std::vector<TexelSamplerRange> sources;
        TexelSamplerRange rangeForSubBlockDigit(GLuint digit) {
            return sources[digit];
        }
    };
    std::string shaderKey() const;
#pragma mark - FFT drawable parameters
    std::vector<SubBlock> subBlocks();
    std::vector<TexelSamplerRange> phaseCorrectSubBlocks();
#pragma mark - Shader creation
    std::string vertShaderSource();
    std::string fragShaderSource();
#pragma mark - Phase table creation and managment
    static void preparePhaseTable(GLuint sigSize, int sign);
    static ISTextureRef renderPhaseTable(GLuint sigSize, int sign);
    ISTextureRef getPhaseTable();
    void bindPhaseTable(GLuint textureUnit);
    void releaseCache();
#pragma mark - ISDrawable implementation

    FFTSubBlock(Orientation orientation, GLuint butterflySize1, GLuint butterflySize2,
                GLuint subBlockLength,
                GLuint subBlockDigitOut, OutType outType, int sign) : ISDrawableT(), _orientation(orientation), _butterflySize1(butterflySize1), _butterflySize2(butterflySize2), _subBlockLength(subBlockLength), _subBlockDigitOut(subBlockDigitOut), _sigSize(0), _outType(outType), _inputReUP(0), _inputImUP(0), _phaseCorrectTableUP(0), _isPhaseCorrecting(false), _sign(sign) { }
    void init() {
        assert(_targetROI.size() == _sourceROI.size());
        _sigSize = size();
        _isPhaseCorrecting = _subBlockLength*_butterflySize1 != _sigSize;
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
        if (_isPhaseCorrecting) {
            bindPhaseTable(inputTuple->textureUnitsUsed());
        }
    }
    void glMonadEscapeHatch() {
        preparePhaseTable(_sigSize, _sign);
    }
    size_t hashImpl() const {
        size_t result = std::hash<GLuint>()(_butterflySize1);
        result ^= std::hash<GLuint>()(_butterflySize2);
        result ^= std::hash<GLuint>()(_subBlockLength);
        result ^= std::hash<GLuint>()(_subBlockDigitOut);
        result ^= std::hash<GLuint>()(_sigSize);
        result ^= std::hash<GLuint>()(outType());
        result ^= std::hash<int>()(_sign);
        if (_orientation == Orientation::Cols) {
            result ^= std::hash<size_t>()(0);
        } else {
            result ^= std::hash<size_t>()(1);
        }
        return result;
    }
    bool compareImpl(const FFTSubBlock& rhs) const {
        bool result = _butterflySize1 == rhs._butterflySize1;
        result &= _butterflySize2 == rhs._butterflySize2;
        result &= _subBlockLength == rhs._subBlockLength;
        result &= _subBlockDigitOut == rhs._subBlockDigitOut;
        result &= _sigSize == rhs._sigSize;
        result &= _outType == rhs._outType;
        result &= _sign == rhs._sign;
        result &= _orientation == rhs._orientation;
        return result;
    };
    void drawImpl() { };
    void setupGeometry() {
        ISVertexArray* geometry = new ISVertexArray();
        //TODO: use ISTextureIndexer here, make sure ALL tests that touch an FFT are passed before using.
        //TODO: this does not support ROI
        std::vector<GLfloat> vertices;
        
        std::vector<SubBlock> subBlockList = subBlocks();
        assert(subBlockList.front().sources.size() == _butterflySize1);
        std::vector<TexelSamplerRange> pcs;
        if (_isPhaseCorrecting) {
            pcs = phaseCorrectSubBlocks();
            assert(subBlockList.size() == pcs.size());
        }
        for (int i = 0; i < subBlockList.size(); i++) {
            std::vector<GLfloat> coords1, coords2;
            for (TexelSamplerRange& sampler : subBlockList[i].sources) {
                
                coords1.push_back(sampler.start);
                coords2.push_back(sampler.finish);
            }
            assert(coords1.size() == _butterflySize1);
            assert(coords2.size() == _butterflySize1);

            if (_orientation == Orientation::Cols) {
                if (_isPhaseCorrecting) {
                    coords1.push_back(pcs[i].start/_sigSize);
                    coords2.push_back(pcs[i].finish/_sigSize);
                }
                makeGlLookupColumnVarying(vertices, length(), subBlockList[i].start, subBlockList[i].finish, coords1, coords2);
            } else {
                if (_isPhaseCorrecting) {
                    appendGlLookupRow(vertices, length(), 1.0f, subBlockList[i].start,
                                      subBlockList[i].finish,
                                      coords1, coords2,
                                      {pcs[i].start/_sigSize, 0.5},
                                      {pcs[i].finish/_sigSize, 0.5});
                } else {
                    appendGlLookupRow(vertices, length(), 1.0f, subBlockList[i].start,
                                      subBlockList[i].finish,
                                      coords1, coords2,
                                      {},
                                      {});
                }

            }
        }
#ifdef DEBUG
        if(_isPhaseCorrecting) {
            assert(vertices.size() == subBlockList.size()*6*(3 + 2*_butterflySize1 + 2));
        } else {
            assert(vertices.size() == subBlockList.size()*6*(3 + 2*_butterflySize1));
        }
#endif
        
        // Layout {x, y, z, i0u, i0v, i1u, i1v, ... pu, pv}
        geometry->addFloatAttribute(0, 3); //positionIn
        GLuint offset = 1;
        for (GLuint i = 0; i < _butterflySize1; i++) {
            geometry->addFloatAttribute(offset, 2); //iIn
            offset++;
        }
        if (_isPhaseCorrecting) {
            geometry->addFloatAttribute(offset, 2); //phaseCorrectIn
        }
        geometry->upload(vertices);
        _geometry = geometry;
    }
    void setupShaderProgram(ISShaderProgram* program) {
        //TODO: 2 point butterfly is the same for sign, make cache lookups ignore the sign for 2 point
        if (!program->tryLoadFromKey(shaderKey())) {
            //No shader in cache, make it
            std::vector<ShaderAttribute> attributeMap{
                {0, "positionIn"},
            };
            GLuint offset = 1;
            for (GLuint i = 0; i < _butterflySize1; i++) {
                std::string s("iIn");
                s.append(std::to_string(i));
                attributeMap.push_back({offset, s});
                offset++;
            }
            if (_isPhaseCorrecting) {
                attributeMap.push_back({offset, "phaseCorrectIn"});
            }
            std::string vertShader = vertShaderSource();
            program->loadShader(fragShaderSource(), prependOrthoMatrixUniform(vertShader), attributeMap);
        }
    }
    void resolveUniformPositions() {
        _inputReUP = glGetUniformLocation(_program->program(), "inputRe");
        _inputImUP = glGetUniformLocation(_program->program(), "inputIm");
        if (_isPhaseCorrecting) {
            _phaseCorrectTableUP = glGetUniformLocation(_program->program(), "phaseCorrectTable");
        }
    }
    
protected:
    static std::unordered_map<int, ISTextureRef> _phaseTableCache;
    
    GLuint _inputReUP;
    GLuint _inputImUP;
    GLuint _phaseCorrectTableUP;
    
    OutType _outType;
    GLuint _butterflySize1; //R_{M-b}
    GLuint _butterflySize2; //R_{M-b-1}
    GLuint _subBlockLength; //T_{b-1}
    GLuint _subBlockDigitOut; //k_{b-1}
    GLuint _sigSize; //N
    int _sign;
    bool _isPhaseCorrecting;
    
    Orientation _orientation;

};

std::vector<GLuint> factorInteger(GLuint n);
std::vector<GLuint> collectTwos(std::vector<GLuint> factors);

#endif /* defined(__Image_Shader__FFTSubBlock__) */
