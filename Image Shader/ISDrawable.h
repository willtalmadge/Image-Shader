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

#ifndef __Image_Shader__ISDrawable__
#define __Image_Shader__ISDrawable__

#include <iostream>
#include <unordered_set>
#include <typeinfo>
#include "ISShaderProgram.h"
#include "ISVertexArray.h"
#include "ISRect.h"

#include "assert.h"
#include "Macro.hpp"
#include <vector>
#include <GLKit/GLKMatrix4.h>

struct IISDrawable {
    size_t hash() const {
        size_t code = typeid(*this).hash_code() ^ baseHash();
        return code;
    }
    bool operator==(const IISDrawable& rhs) const {
        return typeid(*this) == typeid(rhs) &&
               baseCompare(rhs);
        
    }
    virtual void deleteDrawable() const = 0;
    virtual ~IISDrawable() {
        
    }
protected:
    virtual size_t baseHash() const = 0;
    virtual bool baseCompare(const IISDrawable& rhs) const = 0;
};

typedef const IISDrawable* ISDrawableRef;

struct ISDrawableCache {
    static void releaseCache() {
        for (ISDrawableRef drawable : ISDrawableCache::_drawableCache) {
            drawable->deleteDrawable();
            delete drawable;
        }
        _drawableCache.clear();
    }
    struct drawableHash {
        size_t operator()(const ISDrawableRef& drawable) {
            size_t code = drawable->hash();
            return code;
        }
    };
    struct drawableCompare {
        bool operator()(const ISDrawableRef& lhs, const ISDrawableRef& rhs) {
            return *lhs == *rhs;
        }
    };
    static std::unordered_set<ISDrawableRef, drawableHash, drawableCompare> _drawableCache;
};

template<class TupleInT, class TupleOutT, class DrawableT>
struct ISDrawable : public IISDrawable {
    void bind(TupleInT* inputTuple, TupleOutT* outputTuple) {
        assert(!_isBound); //Don't rebind before drawing
        if (!_isSetup) {
            setup();
        }
        glUseProgram(_program->program());
        inputTuple->bind(static_cast<const DrawableT*>(this));
        glUniformMatrix4fv(_orthoMatrixPosition, 1, false, _orthoMatrix.m);
        bindUniforms(inputTuple, outputTuple);
        _isBound = true;
    }
    void draw() {
        assert(_isBound); //Don't draw to something you haven't bound
        _geometry->draw();
        drawImpl();
        _isBound = false; //Don't double draw
    }
    size_t baseHash() const {
        size_t result = std::hash<unsigned int>()(_targetSize.width());
        result ^=  std::hash<unsigned int>()(_targetSize.height());
        result ^= std::hash<unsigned int>()(_sourceROI.left());
        result ^= std::hash<unsigned int>()(_sourceROI.right());
        result ^= std::hash<unsigned int>()(_sourceROI.top());
        result ^= std::hash<unsigned int>()(_sourceROI.bottom());
        result ^= std::hash<unsigned int>()(_targetROI.left());
        result ^= std::hash<unsigned int>()(_targetROI.right());
        result ^= std::hash<unsigned int>()(_targetROI.top());
        result ^= std::hash<unsigned int>()(_targetROI.bottom());
        result ^= hashImpl();
        return result;
    }
    bool baseCompare(const IISDrawable& rhs) const {
        bool result =_targetSize == static_cast<const DrawableT&>(rhs)._targetSize;
        result &= _sourceROI == static_cast<const DrawableT&>(rhs)._sourceROI;
        result &= _targetROI == static_cast<const DrawableT&>(rhs)._targetROI;
        result &= compareImpl(static_cast<const DrawableT&>(rhs));
        return result;
    }
    void setup() {
        assert(static_cast<ISDrawableRef>(this) == dynamic_cast<ISDrawableRef>(this));
        DLPRINT("Looking for drawable %zu\n", static_cast<ISDrawableRef>(this)->hash());
        auto it = ISDrawableCache::_drawableCache.find(static_cast<ISDrawableRef>(this));
        if (it != ISDrawableCache::_drawableCache.end()) {
            DLPRINT("Drawable cache hit %zu\n", static_cast<ISDrawableRef>(this)->hash());
            _program = dynamic_cast<const ISDrawable*>(*it)->_program;
            _geometry = dynamic_cast<const ISDrawable*>(*it)->_geometry;
            resolveUniformPositions(); //FIXME: try to find a way to copy rather than resolve again
            delete (*it);
            ISDrawableCache::_drawableCache.erase(it);
        }
        else {
            assert(_sourceROI.right() > 0);
            assert(_sourceROI.bottom() > 0);
            assert(_targetROI.right() > 0);
            assert(_targetROI.bottom() > 0);
            setupGeometry();
            ISShaderProgram* program = new ISShaderProgram();
            setupShaderProgram(program);
            _program = program;
            _orthoMatrixPosition = glGetUniformLocation(_program->program(), "orthoMatrix");
            resolveUniformPositions();
        }
        _isSetup = true;
    }
    void cache() {
        DrawableT* copy = new DrawableT(static_cast<DrawableT&>(*this));
        DLPRINT("Inserting drawable into cache %zu\n", copy->hash());
        ISDrawableCache::_drawableCache.insert(copy);
    }
    void deleteDrawable() const {
        _geometry->deleteArray();
        //TODO: cleanup shaders and programs
    }
    virtual void glMonadEscapeHatch() {
        /*default implementation does nothing*/
        /*overload to put things that use GL outside the monadic pipeline*/
        /*no state is managed for you here, must do everything manually*/
        /*pipeline gl state is restored after this method is called*/
        /*use with care*/};
    
    ISDrawable() : _isSetup(false), _program(NULL), _geometry(NULL), _isBound(false), _orthoMatrix(GLKMatrix4Identity), _orthoMatrixPosition(0), _targetSize(ISSize(0, 0)), _sourceROI(ISRect(0,0,0,0)), _targetROI(ISRect(0,0,0,0)) { };
    virtual ~ISDrawable() {

    }
    void setRenderMetrics(ISSize sourceSize, ISSize targetSize, ISRect sourceROI, ISRect targetROI) {
        //The pipeline calls this before anything else in the drawable becomes active.
        _sourceSize = sourceSize;
        _targetSize = targetSize;
        _orthoMatrix = GLKMatrix4MakeOrtho(0.0, targetSize.width(), 0.0, targetSize.height(), -1.0, 1.0);
        _sourceROI = sourceROI;
        _targetROI = targetROI;
        init();
    }
    uint texelWidth() const {
        return _sourceSize.width();
    }
    uint texelHeight() const {
        return _sourceSize.height();
    }
protected:
    std::string prependOrthoMatrixUniform(const std::string& vertShader) {
        std::string result("uniform mat4 orthoMatrix;\n");
        result += vertShader;
        return result;
    }
    virtual void bindUniforms(TupleInT* inputTuple, TupleOutT* outputTuple) = 0;
    virtual void drawImpl() = 0;
    virtual size_t hashImpl() const = 0; //Implementer: only hash non-uniform input parameters unique to the class
    virtual bool compareImpl(const DrawableT& rhs) const = 0;
    virtual void init() = 0; //Called after render metrics are set to finish initialization that couldn't be done in the constructor
    
    virtual void setupGeometry() = 0;
    virtual void setupShaderProgram(ISShaderProgram* program) = 0;
    virtual void resolveUniformPositions() = 0;
    
    bool _isBound;
    bool _isSetup;
    GLuint _orthoMatrixPosition;
    GLKMatrix4 _orthoMatrix;
    ISSize _sourceSize;
    ISSize _targetSize;
    ISRect _sourceROI;
    ISRect _targetROI;
    const ISShaderProgram* _program;
    const ISVertexArray* _geometry;

};

//TODO: improve the semantics of these functions. They are confusing to use and read in code
std::vector<GLfloat> makeGlAttributePixelColumn(GLuint length, GLuint x1, GLuint x2,
                                                const std::vector<GLfloat>& attributesTop,
                                                const std::vector<GLfloat>& attributesBottom);
std::vector<GLfloat> makeGlAttributePixelRow(GLuint length, GLuint y1, GLuint y2,
                                             const std::vector<GLfloat>& attributesTop,
                                             const std::vector<GLfloat>& attributesBottom);

void makeGlLookupColumnVarying(std::vector<GLfloat>& vec, GLuint length, GLuint x1, GLuint x2,
                               const std::vector<GLfloat>& u1s,
                               const std::vector<GLfloat>& u2s);
void appendGlLookupCol(std::vector<GLfloat>& vec, GLuint length, GLuint x1, GLuint x2,
                       const std::vector<GLfloat>& u1s,
                       const std::vector<GLfloat>& u2s,
                       const std::vector<GLfloat>& attr1,
                       const std::vector<GLfloat>& attr2);
void appendGlLookupRow(std::vector<GLfloat>& vec, GLuint length, GLfloat textureLength, GLuint y1, GLuint y2,
                            const std::vector<GLfloat>& v1s,
                            const std::vector<GLfloat>& v2s,
                            const std::vector<GLfloat> attr1,
                            const std::vector<GLfloat> attr2);
void makeGlAttributeColumnVarying(std::vector<GLfloat>& vec, GLuint length, GLuint x1, GLuint x2,
                                  const std::vector<GLfloat>& attributes1,
                                  const std::vector<GLfloat>& attributes2);
#endif /* defined(__Image_Shader__ISDrawable__) */
//ISDrawable lives for the duration of the pipeline in the cache. They are only
//swept away when the pipeline is torn down at the end.
//TODO: make cache cleanup for all caching objects.