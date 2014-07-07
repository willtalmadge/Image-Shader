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

#include "ISVertexArray.h"
#include "assert.h"
#include "Macro.hpp"

using namespace std;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
GLuint VertAttributeDescriptor::length()
{
    switch (type) {
        case GL_FLOAT:
            return size*sizeof(GLfloat);
            break;
        default:
            return 0;
            break;
    }
}

ISVertexArray::ISVertexArray() : isSetup(false), drawType(GL_TRIANGLES) { }

GLuint ISVertexArray::vertexSize()
{
    GLuint offset = 0;
    for (VertAttributeDescriptor attrDesc : attributes)
    {
        offset += attrDesc.length();
    }
    return offset;
}

void ISVertexArray::addFloatAttribute(GLuint index, GLuint size)
{
    VertAttributeDescriptor newAttr;
    newAttr.position = index;
    newAttr.size = size;
    newAttr.type = GL_FLOAT;
    attributes.push_back(newAttr);
}

void ISVertexArray::upload(const uchar* data, size_t length)
{
    assert(attributes.size() > 0); //Did you forget to describe vertex attributes before upload?
    assert(length > 0); //Did you remember to set the geometry data?
    if (!isSetup) {
        vertexCount = (GLuint)length/vertexSize();
        glGenVertexArraysOES(1, &vertexArray);
        glBindVertexArrayOES(vertexArray);
        
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
        
        GLuint stride = vertexSize();
        GLuint offset = 0;
        for (VertAttributeDescriptor attrDesc : attributes) {
            glEnableVertexAttribArray(attrDesc.position);
            glVertexAttribPointer(attrDesc.position, attrDesc.size,
                                  attrDesc.type, GL_FALSE,
                                  stride, BUFFER_OFFSET(offset));
            offset += attrDesc.length();
        }
        glBindVertexArrayOES(0);
        isSetup = true;
        GLenum error = glGetError();
        if (error) {
            printf("ISVertexArray setup error %d", error);
        }
    }
}

void ISVertexArray::draw() const
{
    assert(isSetup); //Did you forget to set _geometry in your setupGeometry() ?
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE){
        glBindVertexArrayOES(vertexArray);
        glDrawArrays(drawType, 0, vertexCount);
        GLenum error = glGetError();
        if (error) {
            printf("ISVertexArray draw error %d", error);
        }
    }
    else {
        printf("GL_FRAMEBUFFER incomplete");
    }
}

void ISVertexArray::deleteArray() const
{
    if (isSetup) {
        GLuint name = vertexBuffer;
        DLPRINT("Deleting array %d\n", name);
        glDeleteBuffers(1, &name);
        glDeleteVertexArraysOES(1, &name);
        isSetup = false;
    }
}

ISVertexArray::~ISVertexArray()
{
    deleteArray();
}