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

#ifndef __Image_Shader__FSTLaplace__
#define __Image_Shader__FSTLaplace__

#include "ISShaderProgram.h"
#include "ISDrawable.h"
#include "ISVertexArray.h"
#include "ISSingleton.h"
#include "ISTextureIndexing.h"
#include "FFTPhaseTable.h"

struct FSTLaplace: public ISDrawable<ISSingleton, ISSingleton, FSTLaplace>, ISSingletonBindable {
    
    
    using ISDrawableT = ISDrawable<ISSingleton, ISSingleton, FSTLaplace>;
    
    //TODO: take arguments to the drawable in the constructor
    FSTLaplace() : ISDrawableT() { }
    void init() { }
    //Implement the methods for the ISBindableType interface i.e. ISSingletonBindable as below
    GLuint textureBindingTarget() const {
        assert(_isSetup);
        return _textureUP;
    }
    void bindUniforms(ISSingleton* inputTuple, ISSingleton* outputTuple) {
        //TODO: set uniforms (not samplers, that is automatic)
    }
    //TODO: hash parameters that determine geometry or shader source (not uniforms)
    size_t hashImpl() const {
        size_t result = 0;
        return result;
        
    };
    //TODO: compare parameters that determine geometry or shader source (not uniforms)
    
    bool compareImpl(const FSTLaplace& rhs) const {
        bool result = true;
        return result;
    };
    void drawImpl() { };
    void setupGeometry() {
        ISVertexArray* geometry = new ISVertexArray();
        std::vector<GLfloat> vertices;
        ISTextureIndexer yWrite, xWrite, yRead, xRead, yMath, xMath;
        yWrite.writes().from(0).upTo(_targetROI.height()).alongCols();  //0
        xWrite.writes().from(0).upTo(_targetROI.width()).alongRows();   //0
        yRead.reads().from(0).upTo(_targetROI.height()).alongCols();    //1
        xRead.reads().from(0).upTo(_targetROI.width()).alongRows();     //1
        yMath.math().from(0).upTo(_targetROI.height()).by(1.0f/(_targetROI.height() - 1.0f)).alongCols();     //2
        xMath.math().from(0).upTo(_targetROI.width()).by(1.0f/(_targetROI.width() - 1.0f)).alongRows();      //2
        indexerGeometry2D(geometry, xWrite, yWrite, {xRead}, {yRead}, {xMath}, {yMath});
        _geometry = geometry;
    }
    void setupShaderProgram(ISShaderProgram* program) {
        std::vector<ShaderAttribute> attributeMap{
            {0, "rWrite"},
            {1, "rRead"},
            {2, "rMath"}
        };
        //TODO: use shader cache for this and all other drawables
        program->loadShader(fragShader, prependOrthoMatrixUniform(vertShader), attributeMap);
    }
    void resolveUniformPositions() {
        _textureUP = glGetUniformLocation(_program->program(), "inputImageTexture");
        //TODO: resolve uniforms and store in UP members
    }
    
protected:
    static const std::string vertShader;
    static const std::string fragShader;
    GLuint _textureUP;
    //TODO: define uniform positions
    
    //TODO: define argument storage
};

#endif /* defined(__Image_Shader__FSTLaplace__) */
