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

#ifndef __Image_Shader__ISPipelineBufferable__
#define __Image_Shader__ISPipelineBufferable__

#include "ISPipeline.h"
#include "ISPBuffer.h"
#include <CoreVideo/CoreVideo.h>
#include <functional>

struct ISPipelineBufferable : public ISPipeline {
    ISPipelineBufferable(std::unique_ptr<ISTextureTuple> value) : ISPipeline(std::move(value)) { };
    ISPipelineBufferable(ISTextureTuple* value) : ISPipeline(value) { };
    //These do not bind for opengl draw operations, behavior with drawables is undefined. Find a way to express this.

    //Transform buffer into a new buffer of the specified type
    template<class InputTupleT, class OutputTupleT, template <class> class OutputTextureT>
    ISPipelineBufferable& transformBuffer(std::function<void (InputTupleT&, OutputTupleT&, ISRect sourceROI, ISRect targetROI)> transformer) {
        assert(_value);
        ISTextureTuple* ptr = _value.release();
        assert(ptr);
        assert(static_cast<InputTupleT*>(ptr) == dynamic_cast<InputTupleT*>(ptr)); //Your template arguments are wrong somewhere
        ptr->map([] (ISTextureRef texture) {
            assert(static_cast<const ISPBuffer*>(texture) == dynamic_cast<const ISPBuffer*>(texture)); //All inputs must be buffered to call this.
        });
        OutputTupleT* output = new OutputTupleT;
        output->template setup<OutputTextureT<ISPBuffer> >(_targetSize.width(), _targetSize.height());
        ptr->map([] (ISTextureRef texture) {
            dynamic_cast<ISPBufferRef>(texture)->bindBaseAddress();
        });
        output->map([] (ISTextureRef texture) {
            dynamic_cast<ISPBufferRef>(texture)->bindBaseAddress();
        });
        glFinish(); //Rendering is not gauranteed to be finished when the base address is bound, if the buffer is accessed prematurely you'll be operating on a buffer actively being rendered to.
        transformer(static_cast<InputTupleT&>(*ptr), *output, _sourceROI, _targetROI);
        ptr->map([] (ISTextureRef texture) {
            dynamic_cast<ISPBufferRef>(texture)->unbindBaseAddress();
        });
        output->map([] (ISTextureRef texture) {
            dynamic_cast<ISPBufferRef>(texture)->unbindBaseAddress();
        });
        output->join(ptr);
        _value = std::unique_ptr<ISTextureTuple>(static_cast<ISTextureTuple*>(output));
        if (_isRoot) {
            delete ptr;
        }
        _targetSize = _value->size();
        defaultROI();
        return *this;
    }
    //Transform buffer in place
    template<class InputTupleT>
    ISPipelineBufferable& transformInPlace(std::function<void (InputTupleT&, ISRect sourceROI, ISRect targetROI)> transformer) {
        assert(_value);
        ISTextureTuple* ptr = _value.release();
        assert(ptr);
        assert(static_cast<InputTupleT*>(ptr) == dynamic_cast<InputTupleT*>(ptr)); //Your template arguments are wrong somewhere
        ptr->map([] (ISTextureRef texture) {
            assert(static_cast<const ISPBuffer*>(texture) == dynamic_cast<const ISPBuffer*>(texture)); //All inputs must be buffered to call this.
        });
        ptr->map([] (ISTextureRef texture) {
            dynamic_cast<ISPBufferRef>(texture)->bindBaseAddress();
        });
        glFinish();
        transformer(static_cast<InputTupleT&>(*ptr), _sourceROI, _targetROI);
        ptr->map([] (ISTextureRef texture) {
            dynamic_cast<ISPBufferRef>(texture)->unbindBaseAddress();
        });
        //Join with self is an identity operation. We assume the transformer has not messed with the tuple. This could be a problem if this were to actually occur. Should check for this, find a way to prohibit or find a way to accomodate it. Although, if the caller wants a new buffer they should call transformBuffer.
        _value = std::unique_ptr<ISTextureTuple>(static_cast<ISTextureTuple*>(ptr));
        _targetSize = _value->size();
        defaultROI();
        return *this;
    }
    virtual ISPipelineBufferable& setTargetSize(uint width, uint height);
    virtual ISPipelineBufferable& setTargetSize(ISSize size);
    virtual ISPipelineBufferable& sourceToTargetSizeDiv(uint widthDiv, uint heightDiv);
    virtual ISPipelineBufferable& sourceToTargetSizeMult(uint widthMult, uint heightMult);
    virtual ISPipelineBufferable& fromROI(ISRect roi);
    virtual ISPipelineBufferable& toROI(ISRect roi);
    virtual ISPipelineBufferable& fullROI();
    
    static void setContext(CVEAGLContext context) { _context = context; };
    static CVEAGLContext _context;
};
#endif /* defined(__Image_Shader__ISPipelineBufferable__) */
