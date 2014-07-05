//
//  FFTPhaseTable.cpp
//  Image Shader
//
//  Created by William Talmadge on 7/1/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#include "FFTPhaseTable.h"
#include <vector>
#include <string>
#include "Macro.hpp"
#include "ISVertexArray.h"
#include "ISDrawable.h"

using namespace std;

unordered_map<int, ISTextureRef> FFTPhaseTable::_phaseTableCache = unordered_map<int, ISTextureRef>();
static string phaseTableVertSource = SHADER_STRING
(
 attribute vec4 positionIn;
 attribute highp float kIn;
 
 uniform mat4 orthoMatrix;
 
 varying highp float k;
 void main()
 {
     gl_Position = orthoMatrix*positionIn;
     k = kIn;
 }
 );
static string phaseTableFragSource = SHADER_STRING
(
 
 varying highp float k;
 uniform highp float N;
 uniform highp float s;
 
 void main()
 {
     highp vec4 result;
     result.r = cos(6.283185307*(k-0.5)/N);
     result.g = s*sin(6.283185307*(k-0.5)/N);
     result.ba = vec2(1.0, 1.0);
     gl_FragColor = result;
 }
 );
void printPhaseTable(GLuint sigSize)
{
    GLfloat *r = new GLfloat[4*sigSize];
    glReadPixels(0, 0, sigSize, 1, GL_RGBA, GL_FLOAT, r);
    for (int i = 0; i < sigSize*4; i++) {
        cout << r[i];
        cout << ", ";
    }
    delete [] r;
}
ISTextureRef FFTPhaseTable::renderPhaseTable(GLuint sigSize, int sign)
{
    
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glViewport(0, 0, sigSize, 64);
    ISTextureRef table = ISRe16Rgba::make(sigSize, 64);
    table->attachToFramebuffer(framebuffer);
    ISShaderProgram tableProgram;
    tableProgram.loadShader(phaseTableFragSource, phaseTableVertSource, {
        {0, "positionIn"},
        {1, "kIn"}
    });
    vector<GLfloat> vertices;
    makeGlAttributeColumnVarying(vertices, 64.0, 0, sigSize, {0.0}, {static_cast<GLfloat>(sigSize)});
    ISVertexArray vertexArray;
    vertexArray.addFloatAttribute(0, 3);
    vertexArray.addFloatAttribute(1, 1);
    vertexArray.upload(vertices);
    glUseProgram(tableProgram.program());
    GLuint orthoMatrixUP = glGetUniformLocation(tableProgram.program(), "orthoMatrix");
    GLuint nUP = glGetUniformLocation(tableProgram.program(), "N");
    GLuint sUP = glGetUniformLocation(tableProgram.program(), "s");
    GLKMatrix4 orthoMatrix = GLKMatrix4MakeOrtho(0.0, sigSize, 0.0, 64.0, -1.0, 1.0);
    glUniformMatrix4fv(orthoMatrixUP, 1, false, orthoMatrix.m);
    glUniform1f(sUP, static_cast<GLfloat>(sign));
    glUniform1f(nUP, static_cast<GLfloat>(sigSize));
    vertexArray.draw();
    table->detach();
    glDeleteFramebuffers(1, &framebuffer);
    _phaseTableCache.insert(pair<int, ISTextureRef>(sign*sigSize, table));
    return table;
}
void FFTPhaseTable::preparePhaseTable(GLuint sigSize, int sign)
{
    DLPRINT("Looking for phase table %u\n", sigSize);
    auto it = _phaseTableCache.find(sign*sigSize);
    if (it == _phaseTableCache.end()) {
        DLPRINT("None found, making and caching a new one\n");
        renderPhaseTable(sigSize, sign);
    }
}
ISTextureRef FFTPhaseTable::getPhaseTable(GLuint sigSize, int sign)
{
    ISTextureRef table = NULL;
    auto it = _phaseTableCache.find(sign*sigSize);
    if (it != _phaseTableCache.end()) {
        table = it->second;
    } else {
        //TODO: find a better way to handle phase tables.
        assert(false); //The table cache should be ready so this path doesn't execute. Call prepare phase table in the appropriate location
        table = renderPhaseTable(sigSize, sign);
    }
    return table;
}
void FFTPhaseTable::releaseCache()
{
    DLPRINT("Releasing cached phase table textures");
    for (pair<GLuint, ISTextureRef> texture : _phaseTableCache) {
        texture.second->deleteTexture();
        delete texture.second;
    }
}