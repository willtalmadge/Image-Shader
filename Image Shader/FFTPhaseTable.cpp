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

#include "FFTPhaseTable.h"
#include <vector>
#include <string>
#include "Macro.hpp"
#include "ISVertexArray.h"
#include "ISDrawable.h"

using namespace std;
namespace std {
    template<>
    struct hash<pair<int, float> > {
        size_t operator()(const pair<int, float>& operand) const {
            size_t result = std::hash<int>()(operand.first);
            result ^= std::hash<float>()(operand.second);
            return result;
        }
    };
}

unordered_map<pair<int, float>, ISTextureRef> FFTPhaseTable::_phaseTableCache = unordered_map<pair<int, float>, ISTextureRef>();
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
    return renderPhaseTable(sigSize, static_cast<GLfloat>(sigSize), sign);
}
ISTextureRef FFTPhaseTable::renderPhaseTable(GLuint sigSize, GLfloat period, int sign)
{
    
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glViewport(0, 0, sigSize, 1);
    ISTextureRef table = ISRe16Rgba::make(sigSize, 1);
    table->attachToFramebuffer(framebuffer);
    ISShaderProgram tableProgram;
    tableProgram.loadShader(phaseTableFragSource, phaseTableVertSource, {
        {0, "positionIn"},
        {1, "kIn"}
    });
    vector<GLfloat> vertices;
    makeGlAttributeColumnVarying(vertices, 1.0, 0, sigSize, {0.0}, {static_cast<GLfloat>(sigSize)});
    ISVertexArray vertexArray;
    vertexArray.addFloatAttribute(0, 3);
    vertexArray.addFloatAttribute(1, 1);
    vertexArray.upload(vertices);
    glUseProgram(tableProgram.program());
    GLuint orthoMatrixUP = glGetUniformLocation(tableProgram.program(), "orthoMatrix");
    GLuint nUP = glGetUniformLocation(tableProgram.program(), "N");
    GLuint sUP = glGetUniformLocation(tableProgram.program(), "s");
    GLKMatrix4 orthoMatrix = GLKMatrix4MakeOrtho(0.0, sigSize, 0.0, 1.0, -1.0, 1.0);
    glUniformMatrix4fv(orthoMatrixUP, 1, false, orthoMatrix.m);
    glUniform1f(sUP, static_cast<GLfloat>(sign));
    glUniform1f(nUP, period);
    vertexArray.draw();
    table->detach();
    glDeleteFramebuffers(1, &framebuffer);
    _phaseTableCache.insert(pair<pair<int, float>, ISTextureRef>(pair<int, float>(sigSize, sign*period), table));
    return table;
}
void FFTPhaseTable::preparePhaseTable(GLuint sigSize, int sign)
{
    DLPRINT("Looking for phase table %u\n", sigSize);
    auto it = _phaseTableCache.find(pair<int, float>(sigSize, sign*sigSize));
    if (it == _phaseTableCache.end()) {
        DLPRINT("None found, making and caching a new one\n");
        renderPhaseTable(sigSize, sign);
    }
}
ISTextureRef FFTPhaseTable::getPhaseTable(GLuint sigSize, GLfloat period, int sign)
{
    ISTextureRef table = NULL;
    auto it = _phaseTableCache.find(pair<int, float>(sigSize, sign*period));
    if (it != _phaseTableCache.end()) {
        table = it->second;
    } else {
        //TODO: find a better way to handle phase tables.
        assert(false); //The table cache should be ready so this path doesn't execute. Call prepare phase table in the appropriate location
        table = renderPhaseTable(sigSize, period, sign);
    }
    return table;
}
ISTextureRef FFTPhaseTable::getPhaseTable(GLuint sigSize, int sign)
{
    ISTextureRef table = getPhaseTable(sigSize, static_cast<GLfloat>(sigSize), sign);
    return table;
}
void FFTPhaseTable::releaseCache()
{
    DLPRINT("Releasing cached phase table textures");
    for (pair<pair<int, float>, ISTextureRef> texture : _phaseTableCache) {
        texture.second->deleteTexture();
        delete texture.second;
    }
}