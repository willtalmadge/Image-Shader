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
    void transform(std::function<void (InputTupleT&, OutputTupleT&)> transformer) {
        assert(_value);
        ISTextureTuple* ptr = _value.release();
        assert(ptr);
        assert(_rootInitialized);
        assert(static_cast<InputTupleT*>(ptr) == dynamic_cast<InputTupleT*>(ptr)); //Catch wrong tuple pointer
        OutputTupleT* output = new OutputTupleT;
        transformer(static_cast<InputTupleT&>(*ptr), static_cast<OutputTupleT&>(*output));
        //FIXME: create an assertion that output is well formed here.
        output->join(ptr);
        _value = std::unique_ptr<ISTextureTuple>(output);
        if (_isRoot) {
            delete ptr; //Only safe to delete if the pipeline value has no chance of branching
        }
    }

    //TODO: supporting pbuffers requires support for rebaseable textures, in turn this requires providing the concrete texture type as a template parameter and not allowing transformers to touch texture creation directly.
    template<class InputTupleT, template <class> class OutputTextureT, class DrawableT>
    ISPipeline& transform(GLuint width, GLuint height, DrawableT&& drawable) {
        assert(_rootInitialized);
        assert(_value);
        ISTextureTuple* ptr = _value.release();
        assert(ptr);
        assert(static_cast<InputTupleT*>(ptr) == dynamic_cast<InputTupleT*>(ptr)); //Your template arguments are wrong somewhere
        std::unique_ptr<ISSingleton> output(new ISSingleton);
        //TODO: build a model for this memory management scheme and try to make it more comprehensible and manifest that it needs to be done this way

        output->setup<OutputTextureT<ISTexture> >(width, height);
        ISSingleton* outputPtr = output.release();
        
        drawable.bind(static_cast<InputTupleT*>(ptr), outputPtr);
        outputPtr->attach();
        glClear(GL_COLOR_BUFFER_BIT);
        drawable.draw();
        output.reset(outputPtr);
        
        output->join(ptr);
        _value = std::unique_ptr<ISTextureTuple>(static_cast<ISTextureTuple*>(output.release()));
        if (_isRoot) {
            delete ptr;
        }
        drawable.cache();
        return *this;
    }
    /////////////////
    //For multipass
    /////////////////

    template<class InputTupleT, template <class> class OutputTextureT>
    ISPipeline& multipassTransform(GLuint width, GLuint height, std::function<void (ISSingleton&, ISPipeline&)> transformer) {
        
        assert(_rootInitialized);
        assert(_value);
        
        ISSingleton* output = new ISSingleton;
        output->setup<OutputTextureT<ISTexture> >(width, height);
        output->attach();
        glClear(GL_COLOR_BUFFER_BIT);
        transformer(static_cast<ISSingleton&>(*output), *this);
        
        ISTextureTuple* ptr = _value.release();
        assert(ptr);
        assert(static_cast<InputTupleT*>(ptr) == dynamic_cast<InputTupleT*>(ptr)); //Catch wrong tuple pointer
        //output->join(ptr);
        _value = std::unique_ptr<ISTextureTuple>(output);
        if (_isRoot) {
            delete ptr; //Only safe to delete if the pipeline value has no chance of branching
        }
        return *this;
    }
    template<class InputTupleT, class DrawableT>
    ISPipeline& drawablePass(ISSingleton& output, DrawableT&& drawable) {
        assert(_rootInitialized);
        assert(_value);
        ISTextureTuple* inputPtr = _value.release();
        assert(inputPtr);
        assert(static_cast<InputTupleT*>(inputPtr) == dynamic_cast<InputTupleT*>(inputPtr)); //Your template arguments are wrong somewhere
        
        drawable.bind(static_cast<InputTupleT*>(inputPtr), &output);
        //Have to bind without reattaching, reattach requires a clear
        drawable.draw();
        _value.reset(inputPtr);
        drawable.cache();
        return *this;
    }
    void setupRoot();
    void teardown();
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
protected:
    //ISPipeline should only be created on the stack
    static void * operator new(size_t);
    static void * operator new [] (size_t);
    
    static bool _rootInitialized;
    
    std::unique_ptr<ISTextureTuple> _value; //FIXME: should be a unique_ptr
    bool _isRoot;
};
#endif /* defined(__Image_Shader__ISPipeline__) */

