//
//  ISDrawable.h
//  Image Shader
//
//  Created by William Talmadge on 5/27/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#ifndef __Image_Shader__ISDrawable__
#define __Image_Shader__ISDrawable__

#include <iostream>
#include <unordered_set>
#include <typeinfo>
#include "ISShaderProgram.h"
#include "ISVertexArray.h"
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
        bindUniforms(inputTuple, outputTuple);
        //outputTuple->attach(); //pipeline now responsible for attaching, needed for multi-pass rendering
        _isBound = true;
    }
    void draw() {
        assert(_isBound); //Don't draw to something you haven't bound
        _geometry->draw();
        drawImpl();
        _isBound = false; //Don't double draw
    }
    size_t baseHash() const {
        return std::hash<unsigned int>()(_width) ^
               std::hash<unsigned int>()(_height) ^
               hashImpl();
    }
    bool baseCompare(const IISDrawable& rhs) const {
        return _width == static_cast<const DrawableT&>(rhs)._width &&
               _height == static_cast<const DrawableT&>(rhs)._height &&
               compareImpl(static_cast<const DrawableT&>(rhs));
    }

    void makeSimpleQuad(GLuint positionShaderLocation, GLuint texCoordShaderLocation) {
        GLfloat w = _width;
        GLfloat h = _height;
        GLfloat gQuadData[30] =
        {
            // Data layout for each line below is:
            // positionX, positionY, positionZ,  texS, texT
            0.0,    0.0,   0.5f,  0.0, 0.0,
            w,      0.0,   0.5f,  1.0, 0.0,
            0.0,    h,     0.5f,  0.0, 1.0,
            
            w,      h,     0.5f,  1.0, 1.0,
            0.0,    h,     0.5f,  0.0, 1.0,
            w,      0.0,   0.5f,  1.0, 0.0
        };
        ISVertexArray* geometry = new ISVertexArray();
        geometry->addFloatAttribute(positionShaderLocation, 3);
        geometry->addFloatAttribute(texCoordShaderLocation, 2);
        geometry->upload((uchar*)gQuadData, sizeof(gQuadData));
        _geometry = geometry;
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
            setupGeometry();
            ISShaderProgram* program = new ISShaderProgram();
            setupShaderProgram(program);
            _program = program;
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
    
    ISDrawable(GLuint width, GLuint height) : _isSetup(false), _width(width), _height(height), _program(NULL), _geometry(NULL), _isBound(false) { };
    virtual ~ISDrawable() {

    }
protected:

    virtual void bindUniforms(TupleInT* inputTuple, TupleOutT* outputTuple) = 0;
    virtual void drawImpl() = 0;
    virtual size_t hashImpl() const = 0; //Implementer: only hash non-uniform input parameters unique to the class
    virtual bool compareImpl(const DrawableT& rhs) const = 0;
    
    virtual void setupGeometry() = 0;
    virtual void setupShaderProgram(ISShaderProgram* program) = 0;
    virtual void resolveUniformPositions() = 0;
    
    bool _isBound;
    bool _isSetup;
    GLuint _width;
    GLuint _height;
    const ISShaderProgram* _program;
    const ISVertexArray* _geometry;
};

std::vector<GLfloat> makeGlAttributePixelColumn(GLuint length, GLuint x1, GLuint x2,
                                                const std::vector<GLfloat>& attributesTop,
                                                const std::vector<GLfloat>& attributesBottom);
std::vector<GLfloat> makeGlAttributePixelRow(GLuint length, GLuint y1, GLuint y2,
                                             const std::vector<GLfloat>& attributesTop,
                                             const std::vector<GLfloat>& attributesBottom);

void makeGlLookupColumnVarying(std::vector<GLfloat>& vec, GLuint length, GLuint x1, GLuint x2,
                               const std::vector<GLfloat>& u1s,
                               const std::vector<GLfloat>& u2s);
void appendGlLookupRow(std::vector<GLfloat>& vec, GLuint length, GLuint y1, GLuint y2,
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