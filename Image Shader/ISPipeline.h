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

#ifndef __Image_Shader__ISPipeline__
#define __Image_Shader__ISPipeline__

#include <iostream>
#include <functional>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <memory>
#include "ISTextureTuple.h"
#include "ISSingleton.h"
#include "ISRect.h"

struct ISPipeline {
    //ISPipelines are stack allocated
    //I'm using potentially confusing terminology because it is descriptive.
    //If I say 'bind' without qualifier I mean bind in the sense of binding
    //memory to some input to make it available for an operation. If I mean
    //monadic bind I will say as much.
    //'Working transforms' those that actually write to the framebuffer, can only
    //output to ISSingleton types because OpenGL can only render to one texture at a time.
    //This is modeled by forming product morphisms that sequence operations and form product types.
    static void releaseAllCaches();

    template<class InputTupleT, class OutputTupleT>
    void transform(std::function<void (InputTupleT&, OutputTupleT&, ISRect, ISRect)> transformer) {
        assert(_value);
        ISTextureTuple* ptr = _value.release();
        assert(ptr);
        assert(_rootInitialized);
        assert(static_cast<InputTupleT*>(ptr) == dynamic_cast<InputTupleT*>(ptr)); //Catch wrong tuple pointer
        OutputTupleT* output = new OutputTupleT;
        ptr->split(output->textureUnitsUsed() - 1);
        transformer(static_cast<InputTupleT&>(*ptr), static_cast<OutputTupleT&>(*output), _sourceROI, _targetROI);
        
        ptr->map([](ISTextureRef tex) {
            for(int i = 0; i < tex->dangling(); i++) {
                tex->glue();
                tex->terminate();
            }
        });
        
        _value = std::unique_ptr<ISTextureTuple>(output);
        if (_isRoot) {
            delete ptr; //Only safe to delete if the pipeline value has no chance of branching
        }
        _targetSize = _value->size();
        _sourceROI = ISSize(_targetSize);
        _targetROI = _sourceROI;
    }

    template<class InputTupleT, template <class> class OutputTextureT, class DrawableT, class OutputTextureBaseT>
    ISPipeline& transform(DrawableT&& drawable) {
        assert(_rootInitialized);
        assert(_value);
        ISTextureTuple* ptr = _value.release();
        ptr->glue();
        assert(ptr);
        assert(static_cast<InputTupleT*>(ptr) == dynamic_cast<InputTupleT*>(ptr)); //Your template arguments are wrong somewhere
        ISSingleton* output = new ISSingleton;
        output->setup<OutputTextureT<OutputTextureBaseT> >(_targetSize.width(), _targetSize.height());
        drawable.setRenderMetrics(ptr->size(), _targetSize, _sourceROI, _targetROI); //TODO: if the drawable is loaded from the cache, is there a problem calling this?
        drawable.bind(static_cast<InputTupleT*>(ptr), output);
        output->attach();
        glClear(GL_COLOR_BUFFER_BIT);
        drawable.draw();
        
        ptr->terminate();
        output->split(1);
        _value = std::unique_ptr<ISTextureTuple>(static_cast<ISTextureTuple*>(output));
        if (_isRoot) {
            delete ptr;
        }
        drawable.cache();
        _targetSize = _value->size();
        defaultROI();
        return *this;
    }
    /////////////////
    //For multipass
    /////////////////

    template<class InputTupleT, template <class> class OutputTextureT, class OutputTextureBaseT>
    ISPipeline& multipassTransform(std::function<void (ISSingleton&, ISPipeline&)> transformer) {
        
        assert(_rootInitialized);
        assert(_value);
        
        ISSingleton* output = new ISSingleton;
        output->setup<OutputTextureT<OutputTextureBaseT> >(_targetSize.width(), _targetSize.height());
        output->attach();
        glClear(GL_COLOR_BUFFER_BIT);
        _value->glue();
        transformer(static_cast<ISSingleton&>(*output), *this);

        ISTextureTuple* ptr = _value.release();
        
        assert(ptr);
        assert(static_cast<InputTupleT*>(ptr) == dynamic_cast<InputTupleT*>(ptr)); //Catch wrong tuple pointer
        ptr->terminate();
        output->split(1);
        _value = std::unique_ptr<ISTextureTuple>(output);
        if (_isRoot) {
            delete ptr; //Only safe to delete if the pipeline value has no chance of branching
        }
        _targetSize = _value->size();
        defaultROI();
        return *this;
    }
    template<class InputTupleT, class DrawableT>
    ISPipeline& drawablePass(ISSingleton& output, DrawableT&& drawable) {
        assert(_rootInitialized);
        assert(_value);
        ISTextureTuple* inputPtr = _value.release();
        assert(inputPtr);
        assert(static_cast<InputTupleT*>(inputPtr) == dynamic_cast<InputTupleT*>(inputPtr)); //Your template arguments are wrong somewhere
        
        drawable.setRenderMetrics(inputPtr->size(), _targetSize, _sourceROI, _targetROI);
        drawable.bind(static_cast<InputTupleT*>(inputPtr), &output);
        //Have to bind without reattaching, reattach requires a clear
        drawable.draw();
        _value.reset(inputPtr);
        drawable.cache();
        _targetSize = _value->size();
        defaultROI();
        return *this;
    }
    virtual void setupRoot();
    virtual void teardown();
    template<class T>
    std::unique_ptr<T> result() {
        //FIXME: this can leak textures if the result is not passed up to an outer context (unused results)
        ISTextureTuple* ptr = _value.release();
        assert(static_cast<T*>(ptr) == dynamic_cast<T*>(ptr));
        std::unique_ptr<T> result(static_cast<T*>(ptr));
        assert(result);
        return std::move(result); //NRVO probably makes this move redundant, try that sometime
    };
    
    ISPipeline(std::unique_ptr<ISTextureTuple> value);
    ISPipeline(ISTextureTuple* value);

    ISPipeline(ISPipeline&& pipeline) : _isRoot(pipeline._isRoot), _value(std::move(pipeline._value)) { };
    virtual ~ISPipeline();
    static GLuint _framebufferName;
    
    virtual ISPipeline& setTargetSize(uint width, uint height);
    virtual ISPipeline& setTargetSize(ISSize size);
    virtual ISPipeline& sourceToTargetSizeDiv(uint widthDiv, uint heightDiv);
    virtual ISPipeline& sourceToTargetSizeMult(uint widthMult, uint heightMult);
    virtual ISPipeline& fullROI();
    virtual ISPipeline& fromROI(ISRect roi);
    virtual ISPipeline& toROI(ISRect roi);
    ISSize targetSize() const;
    ISRect sourceROI() const;
    ISRect targetROI() const;
protected:
    //ISPipeline should only be created on the stack
    static void * operator new(size_t);
    static void * operator new [] (size_t);
    
    static bool _rootInitialized;
    
    std::unique_ptr<ISTextureTuple> _value;
    bool _isRoot;
    
    ISSize _targetSize; //Size of the texture attached to the framebuffer to be drawn to
    ISRect _sourceROI;
    ISRect _targetROI;
    
    void defaultROI();
};
#endif /* defined(__Image_Shader__ISPipeline__) */

