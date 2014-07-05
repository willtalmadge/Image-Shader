//
//  GLShaderProgram.h
//  Image Shader
//
//  Created by William Talmadge on 4/26/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <GLKit/GLKit.h>
#import <vector>
#import <string>

struct ShaderAttribute {
    GLuint position;
    std::string name;
};
@interface GLShaderProgram : NSObject

@property GLuint program;
@property NSString* textureAttributeName;

- (BOOL)loadFragmentShaderFromFile:(NSString*)fsName
              vertexShaderFromFile:(NSString*)vsName
                        attributes:(std::vector<ShaderAttribute>)attributes
              textureAttributeName:(NSString*)textureAttributeName;
- (BOOL)loadFragmentShaderFromSource:(NSString*)fsSource
              vertexShaderFromSource:(NSString*)vsSource
                          attributes:(std::vector<ShaderAttribute>)attributes
                textureAttributeName:(NSString*)textureAttributeName;
- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file;
- (BOOL)validateProgram:(GLuint)prog;
- (void)setUniform1i:(GLuint)value forName:(NSString*)name;
- (void)setSamplerToTextureUnit:(GLuint)textureUnit;
@end
