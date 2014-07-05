//
//  ISShaderProgram.h
//  Image Shader
//
//  Created by William Talmadge on 6/5/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#ifndef __Image_Shader__ISShaderProgram__
#define __Image_Shader__ISShaderProgram__

#include <iostream>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <map>
#include <string>
#include <vector>

#define STRINGIZE(x) #x
#define STRINGIZE2(x) STRINGIZE(x)
#define SHADER_STRING(text) STRINGIZE2(text)

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

struct ShaderAttribute {
    GLuint position;
    std::string name;
};

struct ISShaderProgram {
    static std::map<std::string, ISShaderProgram> shaderCache;
    static bool hasShaderForKey(std::string key);
    static ISShaderProgram getForKey(std::string key);
    static void cacheShader(std::string key, ISShaderProgram program);
    
    ISShaderProgram();
    bool loadShader(std::string fsSource,
                    std::string vsSource,
                    std::vector<ShaderAttribute> attributes);
    bool compileShader(GLuint* shader, GLenum type, std::string source);
    bool linkProgram(GLuint prog);
    bool validateProgram(GLuint prog);
    bool tryLoadFromKey(std::string key);
    GLuint program() const { return _program; }

protected:
    GLuint _program;
};
#endif /* defined(__Image_Shader__ISShaderProgram__) */
