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

#include "FSTFinalize.h"
#include "ISPBuffer.h"
#include "arm_neon.h"

using namespace std;

std::function<void (ISComplex&, ISSingleton&, ISRect, ISRect)> fstFinalize_Re16RgbaVFP4(FSTFinalizeOrientation orientation, GLfloat scale)
{
    //Width is the size of the target buffer, the source buffer must be half this
    //TODO: get an assertion for this assumption here
    
    return [=] (ISComplex& input, ISSingleton& output, ISRect sourceROI, ISRect targetROI) {
        //TODO: handle ROI with top and left non-zero in all the methods in this file
        assert(targetROI.left() == 0 && sourceROI.left() == 0);
        uint width = targetROI.width();
        uint height = targetROI.height();
        assert((width == 2*sourceROI.width() && height == sourceROI.height()) ||
               (height == 2*sourceROI.height() && width == sourceROI.width()));
        size_t N;
        if (orientation == FSTFinalizeOrientation::Cols) {
            N = targetROI.width();
        } else {
            N = targetROI.height();
        }

        ISPBufferRef real = dynamic_cast<ISPBufferRef>(input.getReal());
        ISPBufferRef imag = dynamic_cast<ISPBufferRef>(input.getImag());
        ISPBufferRef out = dynamic_cast<ISPBufferRef>(output.getTexture());
        
        //Input indexer
        size_t Ni = real->bytesPerRow()/sizeof(float16x4_t);
        assert(real->bytesPerRow() == imag->bytesPerRow());
        auto iidx = [=] (size_t r, size_t c) -> size_t {
            return Ni*r + c;
        };
        //Output indexer
        size_t No = out->bytesPerRow()/sizeof(float16x4_t);
        auto oidx = [=] (size_t r, size_t c) -> size_t {
            return No*r + c;
        };
        
        float16x4_t* fre16 = (float16x4_t*)real->baseAddress();
        float16x4_t* fim16 = (float16x4_t*)imag->baseAddress();
        float16x4_t* F16 = (float16x4_t*)out->baseAddress();

        if (orientation == FSTFinalizeOrientation::Cols) {
            for (size_t i = 0; i < height; i++) {
                
                float32x4_t fre = vcvt_f32_f16( fre16[iidx(i, 0)] );
                
                float32x4_t accum = {0.0f, 0.0f, 0.0f, 0.0f};
                
                fre16[iidx(i, 0)] = vcvt_f16_f32( vmulq_n_f32(fre, 0.5f) );
                fim16[iidx(i, 0)] = vcvt_f16_f32(accum);
                
                for (size_t j = 0; j < N/2; j++) {
                    fre = vcvt_f32_f16( fre16[iidx(i, j)] );
                    float32x4_t fim = vcvt_f32_f16( fim16[iidx(i, j)] );
                    fre = vmulq_n_f32(fre, scale);
                    fim = vmulq_n_f32(fim, scale);
                    accum = vaddq_f32(accum, fre);
                    F16[oidx(i, 2*j)] = vcvt_f16_f32(fim);
                    F16[oidx(i, 2*j + 1)] = vcvt_f16_f32( accum );
                }
            }
        } else {
            //TODO: improve cache coherence by inverting the major axis also use lane loads to iterate over 4 rows simultaneously
            for (size_t i = 0; i < width; i++) {
                
                float32x4_t fre = vcvt_f32_f16( fre16[iidx(0, i)] );
                
                float32x4_t accum = {0.0f, 0.0f, 0.0f, 0.0f};
                
                fre16[iidx(0, i)] = vcvt_f16_f32( vmulq_n_f32(fre, 0.5f) );
                fim16[iidx(0, i)] = vcvt_f16_f32(accum);
                
                for (size_t j = 0; j < N/2; j++) {
                    fre = vcvt_f32_f16( fre16[iidx(j, i)] );
                    float32x4_t fim = vcvt_f32_f16(fim16[iidx(j, i)]);
                    fre = vmulq_n_f32(fre, scale);
                    fim = vmulq_n_f32(fim, scale);
                    accum = vaddq_f32(accum, fre);
                    F16[oidx(2*j, i)] = vcvt_f16_f32(fim);
                    F16[oidx(2*j + 1, i)] = vcvt_f16_f32( accum );
                }
            }
        }
    };
}