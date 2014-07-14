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
#include "ISSingleton.h"
#include "ISPBuffer.h"
#include <CoreVideo/CoreVideo.h>
#include <functional>

struct ISPipelineBufferable : public ISPipeline {
    ISPipelineBufferable(std::unique_ptr<ISTextureTuple> value) : ISPipeline(std::move(value)) { };
    ISPipelineBufferable(ISTextureTuple* value) : ISPipeline(value) { };

    template<class InputTupleT, template <class> class OutputTextureT>
    ISPipelineBufferable& transformBuffer(GLuint width, GLuint height,
                                          std::function<void (InputTupleT&, ISSingleton&)> transformer) {
        assert(_value);
        ISTextureTuple* ptr = _value.release();
        assert(ptr);
        assert(static_cast<InputTupleT*>(ptr) == dynamic_cast<InputTupleT*>(ptr)); //Your template arguments are wrong somewhere
        ptr->map([] (ISTextureRef texture) {
            assert(static_cast<const ISPBuffer*>(texture) == dynamic_cast<const ISPBuffer*>(texture)); //All inputs must be buffered to call this.
        });
        ISSingleton* output = new ISSingleton;
        output->setup<OutputTextureT<ISPBuffer> >(width, height);
        ptr->map([] (ISTextureRef texture) {
            dynamic_cast<ISPBufferRef>(texture)->bindBaseAddress();
        });
        output->map([] (ISTextureRef texture) {
            dynamic_cast<ISPBufferRef>(texture)->bindBaseAddress();
        });
        transformer(static_cast<InputTupleT&>(*ptr),  *output);
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
        return *this;
    }
    
    void setContext(CVEAGLContext context) { _context = context; };
    static CVEAGLContext _context;
};
#endif /* defined(__Image_Shader__ISPipelineBufferable__) */
