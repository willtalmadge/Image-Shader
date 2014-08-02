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

#include "ISTextureIndexing.h"
#include "ISVertexArray.h"
#include "assert.h"
using namespace std;

// v0-v1
// | / |
// v3-v2

//Indexes in the direction of rows, oriented in the direction of columns
void appendV0ColPos(vector<GLfloat>& vec, array<GLfloat, 2> range) {
    vec.insert(vec.end(), {range[0], 0.0f, 0.5f});
}
void appendV1ColPos(vector<GLfloat>& vec, array<GLfloat, 2> range) {
    vec.insert(vec.end(), {range[1], 0.0f, 0.5f});
}
void appendV2ColPos(vector<GLfloat>& vec, array<GLfloat, 2> range, GLuint height) {
    vec.insert(vec.end(), {range[1], static_cast<float>(height), 0.5f});
}
void appendV3ColPos(vector<GLfloat>& vec, array<GLfloat, 2> range, GLuint height) {
    vec.insert(vec.end(), {range[0], static_cast<float>(height), 0.5f});
}
//Index in the direction of columns, oriented in the direction of rows
void appendV0RowPos(vector<GLfloat>& vec, array<GLfloat, 2> range) {
    vec.insert(vec.end(), {0.0f, range[0], 0.5f});
}
void appendV1RowPos(vector<GLfloat>& vec, array<GLfloat, 2> range, GLuint width) {
    vec.insert(vec.end(), {static_cast<float>(width), range[0], 0.5f});
}
void appendV2RowPos(vector<GLfloat>& vec, array<GLfloat, 2> range, GLuint width) {
    vec.insert(vec.end(), {static_cast<float>(width), range[1], 0.5f});
}
void appendV3RowPos(vector<GLfloat>& vec, array<GLfloat, 2> range) {
    vec.insert(vec.end(), {0.0f, range[1], 0.5f});
}

//Indexes in the direction of rows, oriented in the direction of columns
template <int V>
void appendColTex(vector<GLfloat>& vec, array<GLfloat, 2> range, bool transpose);
template<> void appendColTex<0>(vector<GLfloat>& vec, array<GLfloat, 2> range, bool transpose) {
    if (transpose) {
        vec.insert(vec.end(), {0.0f, range[0]});
    } else {
        vec.insert(vec.end(), {range[0], 0.0f});
    }
}
template<> void appendColTex<1>(vector<GLfloat>& vec, array<GLfloat, 2> range, bool transpose) {
    if (transpose) {
        vec.insert(vec.end(), {0.0f, range[1]});
    } else {
        vec.insert(vec.end(), {range[1], 0.0f});
    }

}
template<> void appendColTex<2>(vector<GLfloat>& vec, array<GLfloat, 2> range, bool transpose) {
    if (transpose) {
        vec.insert(vec.end(), {1.0f, range[1]});
    } else {
        vec.insert(vec.end(), {range[1], 1.0f});
    }

}
template<> void appendColTex<3>(vector<GLfloat>& vec, array<GLfloat, 2> range, bool transpose) {
    if (transpose) {
        vec.insert(vec.end(), {1.0f, range[0]});
    } else {
        vec.insert(vec.end(), {range[0], 1.0f});
    }
    
}
//Index in the direction of columns, oriented in the direction of rows
template <int V>
void appendRowTex(vector<GLfloat>& vec, array<GLfloat, 2> range, bool transpose);
template<> void appendRowTex<0>(vector<GLfloat>& vec, array<GLfloat, 2> range, bool transpose) {
    if (transpose) {
        vec.insert(vec.end(), {range[0], 0.0f});
    } else {
        vec.insert(vec.end(), {0.0f, range[0]});
    }
    
}
template<> void appendRowTex<1>(vector<GLfloat>& vec, array<GLfloat, 2> range, bool transpose) {
    if (transpose) {
        vec.insert(vec.end(), {range[0], 1.0f});
    } else {
        vec.insert(vec.end(), {1.0f, range[0]});
    }
    
}
template<> void appendRowTex<2>(vector<GLfloat>& vec, array<GLfloat, 2> range, bool transpose) {
    if (transpose) {
        vec.insert(vec.end(), {range[1], 1.0f});
    } else {
        vec.insert(vec.end(), {1.0f, range[1]});
    }
    
}
template<> void appendRowTex<3>(vector<GLfloat>& vec, array<GLfloat, 2> range, bool transpose) {
    if (transpose) {
        vec.insert(vec.end(), {range[1], 0.0f});
    } else {
        vec.insert(vec.end(), {0.0f, range[1]});
    }
    
}
//Two dimensional indexer geometry positions
void appendV0QuadPos(vector<GLfloat>& vec, array<GLfloat, 2> rangeX, array<GLfloat, 2> rangeY) {
    vec.insert(vec.end(), {rangeX[0], rangeY[0], 0.5f});
}
void appendV1QuadPos(vector<GLfloat>& vec, array<GLfloat, 2> rangeX, array<GLfloat, 2> rangeY) {
    vec.insert(vec.end(), {rangeX[1], rangeY[0], 0.5f});
}
void appendV2QuadPos(vector<GLfloat>& vec, array<GLfloat, 2> rangeX, array<GLfloat, 2> rangeY) {
    vec.insert(vec.end(), {rangeX[1], rangeY[1], 0.5f});
}
void appendV3QuadPos(vector<GLfloat>& vec, array<GLfloat, 2> rangeX, array<GLfloat, 2> rangeY) {
    vec.insert(vec.end(), {rangeX[0], rangeY[1], 0.5f});
}
//Two dimensional geometry texture coords
template <int V>
void appendQuadTex(vector<GLfloat>& vec, array<GLfloat, 2> range, array<GLfloat, 2> rangeY, bool transpose);
template<> void appendQuadTex<0>(vector<GLfloat>& vec, array<GLfloat, 2> rangeX, array<GLfloat, 2> rangeY, bool transpose) {
    if (!transpose) {
        vec.insert(vec.end(), {rangeX[0], rangeY[0]});
    } else {
        vec.insert(vec.end(), {rangeY[0], rangeX[0]});
    }
}
template<> void appendQuadTex<1>(vector<GLfloat>& vec, array<GLfloat, 2> rangeX, array<GLfloat, 2> rangeY, bool transpose) {
    if (!transpose) {
        vec.insert(vec.end(), {rangeX[1], rangeY[0]});
    } else {
        vec.insert(vec.end(), {rangeY[0], rangeX[1]});
    }
}
template<> void appendQuadTex<2>(vector<GLfloat>& vec, array<GLfloat, 2> rangeX, array<GLfloat, 2> rangeY, bool transpose) {
    if (!transpose) {
        vec.insert(vec.end(), {rangeX[1], rangeY[1]});
    } else {
        vec.insert(vec.end(), {rangeY[1], rangeX[1]});
    }
}
template<> void appendQuadTex<3>(vector<GLfloat>& vec, array<GLfloat, 2> rangeX, array<GLfloat, 2> rangeY, bool transpose) {
    if (!transpose) {
        vec.insert(vec.end(), {rangeX[0], rangeY[1]});
    } else {
        vec.insert(vec.end(), {rangeY[1], rangeX[0]});
    }
}

ISTextureIndexer& ISTextureIndexer::from(int rangeStart) {
    _rangeStart = rangeStart;
    return *this;
}
ISTextureIndexer& ISTextureIndexer::upTo(int rangeEnd) {
    //This is not inclusive, if the for loop has a condition i < N, then rangedEnd should be N, if it has i <= N, then rangeEnd should be N+1
    _rangeEnd = rangeEnd;
    if (_rangeEnd < _rangeStart && _stride > 0) {
        _stride *= -1.0f;
    }
    return *this;
}
ISTextureIndexer& ISTextureIndexer::by(float stride) {
    _stride = stride;
    return *this;
}
ISTextureIndexer& ISTextureIndexer::offset(int offset) {
    _offset = offset;
    return *this;
}
ISTextureIndexer& ISTextureIndexer::alongRows(GLuint texelCount) {
    _direction = ISDirection::Rows;
    _texelCount = texelCount;
    return *this;
}
ISTextureIndexer& ISTextureIndexer::alongRows() {
    _direction = ISDirection::Rows;
    _texelCount = _rangeEnd - _rangeStart;
    return *this;
}
ISTextureIndexer& ISTextureIndexer::alongCols(GLuint texelCount) {
    _direction = ISDirection::Cols;
    _texelCount = texelCount;
    return *this;
}
ISTextureIndexer& ISTextureIndexer::alongCols() {
    _direction = ISDirection::Cols;
    _texelCount = _rangeEnd - _rangeStart;
    return *this;
}
ISTextureIndexer& ISTextureIndexer::reads() {
    _usage = Usage::Read;
    return *this;
}
ISTextureIndexer& ISTextureIndexer::writes() {
    _usage = Usage::Write;
    return *this;
}
ISTextureIndexer& ISTextureIndexer::math() {
    _usage = Usage::Math;
    return *this;
}
bool ISTextureIndexer::isPosition() {
    return _usage == Usage::Write;
}
bool ISTextureIndexer::isColOriented() {
    return (_direction == ISDirection::Rows);
}
GLuint ISTextureIndexer::texelCount() {
    return _texelCount;
}
ISDirection ISTextureIndexer::direction() {
    return _direction;
}
array<GLfloat, 2> ISTextureIndexer::positionRange() {
    array<GLfloat, 2> result{
        _stride*(_rangeStart - 0.5f) + _offset + 0.5f,
        _stride*(_rangeEnd - 0.5f) + _offset + 0.5f};
    return result;
}
array<GLfloat, 2> ISTextureIndexer::texelRange(size_t scale) {
    array<GLfloat, 2> result{
        (_stride*(_rangeStart - 0.5f) + _offset + 0.5f)/scale,
        (_stride*(_rangeEnd - 0.5f) + _offset + 0.5f)/scale};
    return result;
};
array<GLfloat, 2> ISTextureIndexer::mathRange() {
    array<GLfloat, 2> result{
        (_stride*(_rangeStart - 0.5f) + _offset),
        (_stride*(_rangeEnd - 0.5f) + _offset)};
    return result;
};

template<int V>
void appendTex(vector<GLfloat>& vec, vector<ISTextureIndexer>& readIndexers, const vector<array<GLfloat, 2> >& texRange, ISDirection writeDirection) {
    for (size_t i = 0; i < readIndexers.size(); i++) {
        if (writeDirection == ISDirection::Rows) {
            appendColTex<V>(vec, texRange[i], readIndexers[i].direction() != writeDirection);
        } else if (writeDirection == ISDirection::Cols) {
            appendRowTex<V>(vec, texRange[i], readIndexers[i].direction() != writeDirection);
        }
    }
}
template<int V>
void appendTex(vector<GLfloat>& vec,
               vector<ISTextureIndexer>& readIndexersX,
               vector<ISTextureIndexer>& readIndexersY,
               const vector<array<GLfloat, 2> >& texRangeU,
               const vector<array<GLfloat, 2> >& texRangeV,
               ISDirection xWriteDirection) {
    assert(readIndexersX.size() == readIndexersY.size());
    for (size_t i = 0; i < readIndexersX.size(); i++) {
        appendQuadTex<V>(vec, texRangeU[i], texRangeV[i], readIndexersX[i].direction() != xWriteDirection);
    }
}
template<int V>
void appendMath(vector<GLfloat>& vec,
               vector<ISTextureIndexer>& mathIndexersX,
               vector<ISTextureIndexer>& mathIndexersY,
               const vector<array<GLfloat, 2> >& mathRangeX,
               const vector<array<GLfloat, 2> >& mathRangeY,
               ISDirection xWriteDirection) {
    assert(mathIndexersX.size() == mathIndexersY.size());
    for (size_t i = 0; i < mathIndexersX.size(); i++) {
        appendQuadTex<V>(vec, mathRangeX[i], mathRangeY[i], mathIndexersX[i].direction() != xWriteDirection);
    }
}

void indexerGeometry(ISVertexArray* geometry, GLuint width, GLuint height, ISTextureIndexer writeIndexer, vector<ISTextureIndexer> readIndexers) {
    vector<GLfloat> vec;
    assert(writeIndexer.isPosition());
    array<GLfloat, 2> posRange = writeIndexer.positionRange();
    
    geometry->addFloatAttribute(0, 3);

    vector<array<GLfloat, 2> > texRange(readIndexers.size());
    for (size_t i = 0; i < readIndexers.size(); i++) {
        assert(readIndexers[i]._usage == ISTextureIndexer::Usage::Read);
        texRange[i] = readIndexers[i].texelRange(readIndexers[i].texelCount());
        geometry->addFloatAttribute(i + 1, 2);
    }
    ISDirection dir = writeIndexer.direction();
    if (writeIndexer.isColOriented()) {
        appendV0ColPos(vec, posRange);
        appendTex<0>(vec, readIndexers, texRange, dir);
        appendV1ColPos(vec, posRange);
        appendTex<1>(vec, readIndexers, texRange, dir);
        appendV3ColPos(vec, posRange, height);
        appendTex<3>(vec, readIndexers, texRange, dir);
        
        appendV1ColPos(vec, posRange);
        appendTex<1>(vec, readIndexers, texRange, dir);
        appendV2ColPos(vec, posRange, height);
        appendTex<2>(vec, readIndexers, texRange, dir);
        appendV3ColPos(vec, posRange, height);
        appendTex<3>(vec, readIndexers, texRange, dir);
    } else {
        appendV0RowPos(vec, posRange);
        appendTex<0>(vec, readIndexers, texRange, dir);
        appendV1RowPos(vec, posRange, width);
        appendTex<1>(vec, readIndexers, texRange, dir);
        appendV3RowPos(vec, posRange);
        appendTex<3>(vec, readIndexers, texRange, dir);
        
        appendV1RowPos(vec, posRange, width);
        appendTex<1>(vec, readIndexers, texRange, dir);
        appendV2RowPos(vec, posRange, width);
        appendTex<2>(vec, readIndexers, texRange, dir);
        appendV3RowPos(vec, posRange);
        appendTex<3>(vec, readIndexers, texRange, dir);
    }
    
    geometry->upload(vec);
}
void indexerGeometry2D(ISVertexArray* geometry, ISTextureIndexer writeIndexerX, ISTextureIndexer writeIndexerY, vector<ISTextureIndexer> readIndexersX, vector<ISTextureIndexer> readIndexersY) {
    indexerGeometry2D(geometry, writeIndexerX, writeIndexerY, readIndexersX, readIndexersY, {}, {});
}
void indexerGeometry2D(ISVertexArray* geometry, ISTextureIndexer writeIndexerX, ISTextureIndexer writeIndexerY, vector<ISTextureIndexer> readIndexersX, vector<ISTextureIndexer> readIndexersY, vector<ISTextureIndexer> mathIndexersX, vector<ISTextureIndexer> mathIndexersY) {
    vector<GLfloat> vec;
    static const ISDirection xWriteDirection = ISDirection::Rows;
    static const ISDirection yWriteDirection = ISDirection::Cols;
    assert(writeIndexerX.isPosition());
    assert(writeIndexerY.isPosition());
    assert(writeIndexerX.direction() == xWriteDirection);
    assert(writeIndexerY.direction() == yWriteDirection);
    assert(readIndexersX.size() == readIndexersY.size());
    array<GLfloat, 2> posRangeX = writeIndexerX.positionRange();
    array<GLfloat, 2> posRangeY = writeIndexerY.positionRange();
    
    geometry->addFloatAttribute(0, 3);
    
    vector<array<GLfloat, 2> > texRangeU(readIndexersX.size());
    vector<array<GLfloat, 2> > texRangeV(readIndexersY.size());
    for (size_t i = 0; i < readIndexersX.size(); i++) {
        assert(readIndexersX[i]._usage == ISTextureIndexer::Usage::Read);
        assert(readIndexersY[i]._usage == ISTextureIndexer::Usage::Read);
        assert(readIndexersX[i].direction() != readIndexersY[i].direction()); //Topological constraint
        texRangeU[i] = readIndexersX[i].texelRange(readIndexersX[i].texelCount());
        texRangeV[i] = readIndexersY[i].texelRange(readIndexersY[i].texelCount());
        geometry->addFloatAttribute(i + 1, 2);
    }
    vector<array<GLfloat, 2> > mathRangeX(mathIndexersX.size());
    vector<array<GLfloat, 2> > mathRangeY(mathIndexersY.size());
    for (size_t i = 0; i < mathIndexersX.size(); i++) {
        assert(mathIndexersX[i]._usage == ISTextureIndexer::Usage::Math);
        assert(mathIndexersY[i]._usage == ISTextureIndexer::Usage::Math);
        assert(mathIndexersX[i].direction() != mathIndexersY[i].direction()); //Topological constraint
        mathRangeX[i] = mathIndexersX[i].mathRange();
        mathRangeY[i] = mathIndexersY[i].mathRange();
        geometry->addFloatAttribute(i + readIndexersX.size() + 1, 2);
    }
    
    appendV0QuadPos(vec, posRangeX, posRangeY);
    appendTex<0>(vec, readIndexersX, readIndexersY, texRangeU, texRangeV, xWriteDirection);
    appendMath<0>(vec, mathIndexersX, mathIndexersY, mathRangeX, mathRangeY, xWriteDirection);
    appendV1QuadPos(vec, posRangeX, posRangeY);
    appendTex<1>(vec, readIndexersX, readIndexersY, texRangeU, texRangeV, xWriteDirection);
    appendMath<1>(vec, mathIndexersX, mathIndexersY, mathRangeX, mathRangeY, xWriteDirection);
    appendV3QuadPos(vec, posRangeX, posRangeY);
    appendTex<3>(vec, readIndexersX, readIndexersY, texRangeU, texRangeV, xWriteDirection);
    appendMath<3>(vec, mathIndexersX, mathIndexersY, mathRangeX, mathRangeY, xWriteDirection);

    appendV1QuadPos(vec, posRangeX, posRangeY);
    appendTex<1>(vec, readIndexersX, readIndexersY, texRangeU, texRangeV, xWriteDirection);
    appendMath<1>(vec, mathIndexersX, mathIndexersY, mathRangeX, mathRangeY, xWriteDirection);
    appendV2QuadPos(vec, posRangeX, posRangeY);
    appendTex<2>(vec, readIndexersX, readIndexersY, texRangeU, texRangeV, xWriteDirection);
    appendMath<2>(vec, mathIndexersX, mathIndexersY, mathRangeX, mathRangeY, xWriteDirection);
    appendV3QuadPos(vec, posRangeX, posRangeY);
    appendTex<3>(vec, readIndexersX, readIndexersY, texRangeU, texRangeV, xWriteDirection);
    appendMath<3>(vec, mathIndexersX, mathIndexersY, mathRangeX, mathRangeY, xWriteDirection);
    
    geometry->upload(vec);
}