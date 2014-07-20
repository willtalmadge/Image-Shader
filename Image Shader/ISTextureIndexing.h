//
//  ISTextureIndexing.h
//  Image Shader
//
//  Created by William Talmadge on 7/19/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#ifndef __Image_Shader__ISTextureIndexing__
#define __Image_Shader__ISTextureIndexing__

#include <vector>
#include <array>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

enum class ISDirection { Rows, Cols };
class ISVertexArray;
struct ISTextureIndexer {
    ISTextureIndexer() : _stride(1), _offset(0), _rangeStart(0), _rangeEnd(0) { }
    static ISTextureIndexer make() { return ISTextureIndexer(); }
    ISTextureIndexer& from(int rangeStart);
    ISTextureIndexer& to(int rangeEnd);
    ISTextureIndexer& by(int stride);
    ISTextureIndexer& offset(int offset);
    ISTextureIndexer& alongRows(GLuint texelCount);
    ISTextureIndexer& alongCols(GLuint texelCount);
    ISTextureIndexer& reads();
    ISTextureIndexer& writes();
    bool isPosition();
    bool isColOriented();
    GLuint texelCount();
    ISDirection direction();
    std::array<GLfloat, 2> positionRange();
    std::array<GLfloat, 2> texelRange(size_t scale);

protected:
    int _stride;
    int _offset;
    int _rangeStart;
    int _rangeEnd;
    ISDirection _direction;
    bool _reads;
    GLuint _texelCount;
};

void indexerGeometry(ISVertexArray* geometry, GLuint width, GLuint height, ISTextureIndexer writeIndexer, std::vector<ISTextureIndexer> readIndexers);

#endif /* defined(__Image_Shader__ISTextureIndexing__) */
