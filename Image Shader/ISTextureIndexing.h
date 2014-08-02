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

#ifndef __Image_Shader__ISTextureIndexing__
#define __Image_Shader__ISTextureIndexing__

#include <vector>
#include <array>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

enum class ISDirection { Rows, Cols, Undefined };
class ISVertexArray;
struct ISTextureIndexer {
    enum class Usage { Write, Read, Math, Undefined };
    ISTextureIndexer() : _stride(1), _offset(0), _rangeStart(0), _rangeEnd(0), _direction(ISDirection::Undefined), _usage(Usage::Undefined) { }
    static ISTextureIndexer make() { return ISTextureIndexer(); }
    ISTextureIndexer& from(int rangeStart);
    ISTextureIndexer& upTo(int rangeEnd);
    ISTextureIndexer& by(float stride);
    ISTextureIndexer& offset(int offset);
    ISTextureIndexer& alongRows(GLuint texelCount);
    ISTextureIndexer& alongRows();
    ISTextureIndexer& alongCols(GLuint texelCount);
    ISTextureIndexer& alongCols();
    ISTextureIndexer& reads();
    ISTextureIndexer& writes();
    ISTextureIndexer& math();
    bool isPosition();
    bool isColOriented();
    GLuint texelCount();
    ISDirection direction();
    std::array<GLfloat, 2> positionRange();
    std::array<GLfloat, 2> texelRange(size_t scale);
    std::array<GLfloat, 2> mathRange();

    friend void indexerGeometry(ISVertexArray* geometry, GLuint width, GLuint height, ISTextureIndexer writeIndexer, std::vector<ISTextureIndexer> readIndexers);
    friend void indexerGeometry2D(ISVertexArray* geometry, ISTextureIndexer writeIndexerX, ISTextureIndexer writeIndexerY, std::vector<ISTextureIndexer> readIndexersX, std::vector<ISTextureIndexer> readIndexersY);
    friend void indexerGeometry2D(ISVertexArray* geometry, ISTextureIndexer writeIndexerX, ISTextureIndexer writeIndexerY, std::vector<ISTextureIndexer> readIndexersX, std::vector<ISTextureIndexer> readIndexersY, std::vector<ISTextureIndexer> mathIndexersX, std::vector<ISTextureIndexer> mathIndexersY);
protected:
    float _stride;
    int _offset;
    int _rangeStart;
    int _rangeEnd;
    ISDirection _direction;
    Usage _usage;
    GLuint _texelCount;
};

void indexerGeometry(ISVertexArray* geometry, GLuint width, GLuint height, ISTextureIndexer writeIndexer, std::vector<ISTextureIndexer> readIndexers);
void indexerGeometry2D(ISVertexArray* geometry, ISTextureIndexer writeIndexerX, ISTextureIndexer writeIndexerY, std::vector<ISTextureIndexer> readIndexersX, std::vector<ISTextureIndexer> readIndexersY);
void indexerGeometry2D(ISVertexArray* geometry, ISTextureIndexer writeIndexerX, ISTextureIndexer writeIndexerY, std::vector<ISTextureIndexer> readIndexersX, std::vector<ISTextureIndexer> readIndexersY, std::vector<ISTextureIndexer> mathIndexersX, std::vector<ISTextureIndexer> mathIndexersY);
#endif /* defined(__Image_Shader__ISTextureIndexing__) */
