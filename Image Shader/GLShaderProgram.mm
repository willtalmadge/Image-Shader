//
//  GLShaderProgram.m
//  Image Shader
//
//  Created by William Talmadge on 4/26/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#import "GLShaderProgram.hh"
#import "GLTexture.h"
#import "GLTextureFBO.h"

using namespace std;

@implementation GLShaderProgram

- (BOOL)loadFragmentShaderFromFile:(NSString*)fsName
              vertexShaderFromFile:(NSString*)vsName
                        attributes:(vector<ShaderAttribute>)attributes
              textureAttributeName:(NSString*)textureAttributeName
{
    GLuint vertShader, fragShader;
    NSString *vertShaderPathname, *fragShaderPathname;
    _textureAttributeName = textureAttributeName;
    // Create shader program.
    _program = glCreateProgram();
    
    // Create and compile vertex shader.
    vertShaderPathname = [[NSBundle mainBundle] pathForResource:vsName ofType:@"vsh"];
    if (![self compileShader:&vertShader type:GL_VERTEX_SHADER file:vertShaderPathname]) {
        NSLog(@"Failed to compile vertex shader");
        return NO;
    }
    
    // Create and compile fragment shader.
    fragShaderPathname = [[NSBundle mainBundle] pathForResource:fsName ofType:@"fsh"];
    if (![self compileShader:&fragShader type:GL_FRAGMENT_SHADER file:fragShaderPathname]) {
        NSLog(@"Failed to compile fragment shader");
        return NO;
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
    if (![self linkProgram:_program]) {
        NSLog(@"Failed to link program: %d", _program);
        
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
        
        return NO;
    }
    
    // Release vertex and fragment shaders.
    if (vertShader) {
        glDetachShader(_program, vertShader);
        glDeleteShader(vertShader);
    }
    if (fragShader) {
        glDetachShader(_program, fragShader);
        glDeleteShader(fragShader);
    }
    
    return YES;
}
- (BOOL)loadFragmentShaderFromSource:(NSString*)fsSource
              vertexShaderFromSource:(NSString*)vsSource
                          attributes:(vector<ShaderAttribute>)attributes
                textureAttributeName:(NSString*)textureAttributeName
{
    GLuint vertShader, fragShader;
    _textureAttributeName = textureAttributeName;
    // Create shader program.
    _program = glCreateProgram();
    
    // Create and compile vertex shader.
    if (![self compileShader:&vertShader type:GL_VERTEX_SHADER source:vsSource]) {
        NSLog(@"Failed to compile vertex shader");
        return NO;
    }
    
    // Create and compile fragment shader.
    if (![self compileShader:&fragShader type:GL_FRAGMENT_SHADER source:fsSource]) {
        NSLog(@"Failed to compile fragment shader");
        return NO;
    }
    
    // Attach vertex shader to program.
    glAttachShader(_program, vertShader);
    
    // Attach fragment shader to program.
    glAttachShader(_program, fragShader);
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    for (ShaderAttribute pair : attributes)
    {
        glBindAttribLocation(_program, pair.position, pair.name.c_str());
    }
    
    // Link program.
    if (![self linkProgram:_program]) {
        NSLog(@"Failed to link program: %d", _program);
        
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
        
        return NO;
    }
    
    // Release vertex and fragment shaders.
    if (vertShader) {
        glDetachShader(_program, vertShader);
        glDeleteShader(vertShader);
    }
    if (fragShader) {
        glDetachShader(_program, fragShader);
        glDeleteShader(fragShader);
    }
    
    return YES;
}

- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file
{
    NSString* source = [NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil];
    return [self compileShader:shader type:type source:source];
}
- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type source:(NSString *)source
{
    GLint status;
    const GLchar *sourceUtf8;
    
    sourceUtf8 = (GLchar *)[source UTF8String];
    if (!sourceUtf8) {
        NSLog(@"Failed to load vertex shader");
        return NO;
    }
    
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &sourceUtf8, NULL);
    glCompileShader(*shader);
    
#if defined(DEBUG)
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(*shader, logLength, &logLength, log);
        NSLog(@"Shader compile log:\n%s", log);
        free(log);
    }
#endif
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        glDeleteShader(*shader);
        return NO;
    }
    
    return YES;
}

- (BOOL)linkProgram:(GLuint)prog
{
    GLint status;
    glLinkProgram(prog);
    
#if defined(DEBUG)
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program link log:\n%s", log);
        free(log);
    }
#endif
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0) {
        return NO;
    }
    
    return YES;
}


- (BOOL)validateProgram:(GLuint)prog
{
    GLint logLength, status;
    
    glValidateProgram(prog);
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program validate log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == 0) {
        return NO;
    }
    
    return YES;
}

- (void)setUniform1i:(GLuint)value forName:(NSString*)name
{
    //It would seem that this must be set before any framebuffers are attached to the texture
    //Or maybe it's just if an attached FBO is bound?
    GLuint textureUniformLocation = glGetUniformLocation(_program, [name UTF8String]);
    glUniform1i(textureUniformLocation, value);
    GLenum error = glGetError();
    if (error) {
        NSLog(@"setUniform1i error %d", error);
    }
}
- (void)setSamplerToTextureUnit:(GLuint)textureUnit
{
    [self setUniform1i:textureUnit forName:_textureAttributeName];
}
@end
