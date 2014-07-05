//
//  ISPipeline.h
//  Image Shader
//
//  Created by William Talmadge on 5/25/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#ifndef __Image_Shader__ISPipeline__
#define __Image_Shader__ISPipeline__

#include <iostream>
#include <functional>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <memory>
#include "ISTextureTuple.h"
#include <memory>

struct ISPipeline {
    //Must have a method that takes a lambda expression. This expression
    //will pass in the current value in the pipeline. The passed function
    //must return a new object. The transform function will destruct
    //the old value and perform necessary caching. The sequence is
    //join (+) fmap f
    //where join is essentially the destructor
    //ISPipelines are statically allocated
    static void releaseAllCaches();
    /*
    template<class InputTupleT, class OutputTupleT>
    void transform(std::function<OutputTupleT *(InputTupleT *)> transformer) {
        assert(_rootInitialized);
        assert(static_cast<InputTupleT*>(_value) == dynamic_cast<InputTupleT*>(_value)); //Catch wrong tuple pointer
        //this is a monadic bind
        
        OutputTupleT* temp = transformer(static_cast<InputTupleT*>(_value));
        temp->join(_value);
        //Because this is a monad, previous _value is gauranteed to not be needed by this point so it can be deleted.
        delete _value;
        _value = temp;
    }*/
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

    template<class InputTupleT, class OutputTupleT, class DrawableT>
    ISPipeline& transform(std::function<void (InputTupleT&, OutputTupleT&)> transformer, DrawableT&& drawable) {
        assert(_rootInitialized);
        assert(_value);
        ISTextureTuple* ptr = _value.release();
        assert(ptr);
        assert(static_cast<InputTupleT*>(ptr) == dynamic_cast<InputTupleT*>(ptr)); //Your template arguments are wrong somewhere
        std::unique_ptr<OutputTupleT> output(new OutputTupleT);
        
        //FIXME: rework this
        _rootInitialized = false;
        glBindFramebuffer(GL_FRAMEBUFFER, 0); //Make your own framebuffer!
        drawable.glMonadEscapeHatch(); //You're on your own here
        glBindFramebuffer(GL_FRAMEBUFFER, _framebufferName);
        _rootInitialized = true;
        
        transformer(static_cast<InputTupleT&>(*ptr), static_cast<OutputTupleT&>(*output));
        OutputTupleT* outputPtr = output.release();
        
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
    //Should rename this something like "multipassTransform" this method forms product morphisms from drawable passes, it's not bind
    template<class InputTupleT, class OutputTupleT>
    ISPipeline& transform(std::function<void (OutputTupleT&)> outputSetup,
                          std::function<void (OutputTupleT&, ISPipeline&)> transformer) {
        
        assert(_rootInitialized);
        assert(_value);
        
        OutputTupleT* output = new OutputTupleT;
        outputSetup(static_cast<OutputTupleT&>(*output));
        output->attach();
        glClear(GL_COLOR_BUFFER_BIT);
        transformer(static_cast<OutputTupleT&>(*output), *this);
        
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
    template<class InputTupleT, class OutputTupleT, class DrawableT>
    ISPipeline& drawablePass(OutputTupleT& output, DrawableT&& drawable) {
        assert(_rootInitialized);
        assert(_value);
        ISTextureTuple* inputPtr = _value.release();
        assert(inputPtr);
        assert(static_cast<InputTupleT*>(inputPtr) == dynamic_cast<InputTupleT*>(inputPtr)); //Your template arguments are wrong somewhere
        
        /*
        _rootInitialized = false;
        glBindFramebuffer(GL_FRAMEBUFFER, 0); //Make your own framebuffer!
        drawable.glMonadEscapeHatch(); //You're on your own here
        glBindFramebuffer(GL_FRAMEBUFFER, _framebufferName);
        _rootInitialized = true;
        */
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

