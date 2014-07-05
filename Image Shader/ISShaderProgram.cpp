//
//  ISShaderProgram.cpp
//  Image Shader
//
//  Created by William Talmadge on 6/5/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#include "ISShaderProgram.h"

std::map<std::string, ISShaderProgram> ISShaderProgram::shaderCache = std::map<std::string, ISShaderProgram>();

bool ISShaderProgram::hasShaderForKey(std::string key)
{
    std::map<std::string, ISShaderProgram>::iterator it = shaderCache.find(key);
    if (it != shaderCache.end()) {
        return true;
    }
    return false;
}
ISShaderProgram ISShaderProgram::getForKey(std::string key)
{
    std::map<std::string, ISShaderProgram>::iterator it = shaderCache.find(key);
    return it->second;
}
void ISShaderProgram::cacheShader(std::string key, ISShaderProgram program)
{
    shaderCache.insert({key, program});
}
ISShaderProgram::ISShaderProgram()
{ }
bool ISShaderProgram::loadShader(std::string fsSource,
                                 std::string vsSource,
                                 std::vector<ShaderAttribute> attributes)
{
    GLuint vertShader, fragShader;
    // Create shader program.
    _program = glCreateProgram();
    
    // Create and compile vertex shader.
    
    if (!compileShader(&vertShader, GL_VERTEX_SHADER, vsSource)) {
        printf("Failed to compile vertex shader");
        return false;
    }
    
    // Create and compile fragment shader.
    
    if (!compileShader(&fragShader, GL_FRAGMENT_SHADER, fsSource)) {
        printf("Failed to compile fragment shader");
        return false;
    }
    
    // Attach vertex shader to program.
    glAttachShader(_program, vertShader);
    
    // Attach fragment shader to program.
    glAttachShader(_program, fragShader);
    
    // Bind attribute locations. Attributes are in the vertex shader
    // This needs to be done prior to linking.
    for (ShaderAttribute pair : attributes)
    {
        glBindAttribLocation(_program, pair.position, pair.name.c_str());
    }
    
    // Link program.
    
    if (!linkProgram(_program)) {
        printf("Failed to link program: %d", _program);
        
        if (vertShader) {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader) {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (_program) {
            glDeleteProgram(_program);
            _program = 0;
        }
        
        return false;
    }
   /*
    // Release vertex and fragment shaders.
    if (vertShader) {
        glDetachShader(_program, vertShader);
        glDeleteShader(vertShader);
    }
    if (fragShader) {
        glDetachShader(_program, fragShader);
        glDeleteShader(fragShader);
    }*/
    return true;
}
bool ISShaderProgram::compileShader(GLuint *shader, GLenum type, std::string source)
{
    GLint status;
    const GLchar *sourceUtf8;
    
    sourceUtf8 = (GLchar *)source.c_str();
    
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &sourceUtf8, NULL);
    glCompileShader(*shader);
    
#if defined(DEBUG)
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(*shader, logLength, &logLength, log);
        printf("Shader compile log:\n%s", log);
        free(log);
    }
#endif
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        glDeleteShader(*shader);
        return false;
    }
    
    return true;
}
bool ISShaderProgram::linkProgram(GLuint prog)
{
    GLint status;
    glLinkProgram(prog);
    
#if defined(DEBUG)
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        printf("Program link log:\n%s", log);
        free(log);
    }
#endif
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0) {
        return false;
    }
    
    return true;
}
bool ISShaderProgram::validateProgram(GLuint prog)
{
    GLint logLength, status;
    
    glValidateProgram(prog);
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        printf("Program validate log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == 0) {
        return false;
    }
    
    return true;
}
bool ISShaderProgram::tryLoadFromKey(std::string key)
{
    if (hasShaderForKey(key)) {
        ISShaderProgram cached = getForKey(key);
        _program = cached._program;
        return true;
    } else {
        return false;
    }
}