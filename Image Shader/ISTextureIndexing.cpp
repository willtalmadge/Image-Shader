//
//  ISTextureIndexing.cpp
//  Image Shader
//
//  Created by William Talmadge on 7/19/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

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


ISTextureIndexer& ISTextureIndexer::from(int rangeStart) {
    _rangeStart = rangeStart;
    return *this;
}
ISTextureIndexer& ISTextureIndexer::to(int rangeEnd) {
    _rangeEnd = rangeEnd;
    if (_rangeEnd < _rangeStart && _stride > 0) {
        _stride *= -1;
    }
    return *this;
}
ISTextureIndexer& ISTextureIndexer::by(int stride) {
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
ISTextureIndexer& ISTextureIndexer::alongCols(GLuint texelCount) {
    _direction = ISDirection::Cols;
    _texelCount = texelCount;
    return *this;
}
ISTextureIndexer& ISTextureIndexer::reads() {
    _reads = true;
    return *this;
}
ISTextureIndexer& ISTextureIndexer::writes() {
    _reads = false;
    return *this;
}
bool ISTextureIndexer::isPosition() {
    return !_reads;
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

template<int V>
void appendTex(vector<GLfloat>& vec, vector<ISTextureIndexer>& readIndexers, const vector<array<GLfloat, 2> >& texRange, ISDirection writeDirection) {
    for (size_t i = 0; i < readIndexers.size(); i++) {
        if (writeDirection == ISDirection::Rows) {
            appendColTex<V>(vec, texRange[i], readIndexers[i].direction() != writeDirection);
        } else {
            appendRowTex<V>(vec, texRange[i], readIndexers[i].direction() != writeDirection);
        }
    }
}

void indexerGeometry(ISVertexArray* geometry, GLuint width, GLuint height, ISTextureIndexer writeIndexer, vector<ISTextureIndexer> readIndexers) {
    //If tex orientation is opposite orientation of pos orientation the only action to take is to transpose u and v from what it would be if the orientation were matched to pos.
    vector<GLfloat> vec;
    assert(writeIndexer.isPosition());
    array<GLfloat, 2> posRange = writeIndexer.positionRange();
    
    geometry->addFloatAttribute(0, 3);

    vector<array<GLfloat, 2> > texRange(readIndexers.size());
    for (size_t i = 0; i < readIndexers.size(); i++) {
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