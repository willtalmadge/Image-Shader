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
