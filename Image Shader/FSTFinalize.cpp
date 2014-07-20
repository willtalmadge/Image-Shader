//
//  FSTFinalize.cpp
//  Image Shader
//
//  Created by William Talmadge on 7/13/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#include "FSTFinalize.h"
#include "ISPBuffer.h"
#include "arm_neon.h"

using namespace std;

std::function<void (ISComplex&, ISSingleton&)> fstFinalizeForward_Re16RgbaVFP4(GLuint width, GLuint height, FSTFinalizeOrientation orientation)
{
    //Width is the size of the target buffer, the source buffer must be half this
    //TODO: get an assertion for this assumption here
    
    return [=] (ISComplex& input, ISSingleton& output) {
        size_t N;
        if (orientation == FSTFinalizeOrientation::Cols) {
            N = width;
        } else {
            N = height;
        }

        ISPBufferRef real = dynamic_cast<ISPBufferRef>(input.getReal());
        ISPBufferRef imag = dynamic_cast<ISPBufferRef>(input.getImag());
        ISPBufferRef out = dynamic_cast<ISPBufferRef>(output.getTexture());
        
        //Input indexer
        size_t Ni = real->bytesPerRow()/sizeof(float16x4_t);
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
                
                for (size_t j = 0; j < N/2 - 1; j++) {
                    fre = vcvt_f32_f16( fre16[iidx(i, j)] );
                    accum = vaddq_f32(accum, fre);
                    F16[oidx(i, 2*j)] = fim16[iidx(i, j)];
                    F16[oidx(i, 2*j + 1)] = vcvt_f16_f32( accum );
                }
            }
        } else {
            //FIXME: you are peaking into the buffer here, the logical coordinate system is no longer valid, the y-axis runs vertical, the transform might be running backwards when the direction is columns.
            for (size_t i = 0; i < width; i++) {
                
                float32x4_t fre = vcvt_f32_f16( fre16[iidx(0, i)] );
                
                float32x4_t accum = {0.0f, 0.0f, 0.0f, 0.0f};
                
                fre16[iidx(0, i)] = vcvt_f16_f32( vmulq_n_f32(fre, 0.5f) );
                fim16[iidx(0, i)] = vcvt_f16_f32(accum);
                
                for (size_t j = 0; j < N/2 - 1; j++) {
                    fre = vcvt_f32_f16( fre16[iidx(j, i)] );
                    accum = vaddq_f32(accum, fre);
                    F16[oidx(2*j, i)] = fim16[iidx(j, i)];
                    F16[oidx(2*j + 1, i)] = vcvt_f16_f32( accum );
                }
            }
        }
    };
}
std::function<void (ISComplex&, ISSingleton&)> fstFinalize_Re16RgbaVFP4(GLuint width, GLuint height, FSTFinalizeOrientation orientation)
{
    //Width is the size of the target buffer, the source buffer must be half this
    //TODO: get an assertion for this assumption here
    size_t N;
    float k;
    if (orientation == FSTFinalizeOrientation::Cols) {
        N = width;
        k = sqrt(2.0/width);
    } else {
        N = height;
        k = sqrt(2.0/height);
    }
    return [=] (ISComplex& input, ISSingleton& output) {
        
        ISPBufferRef real = dynamic_cast<ISPBufferRef>(input.getReal());
        ISPBufferRef imag = dynamic_cast<ISPBufferRef>(input.getImag());
        ISPBufferRef out = dynamic_cast<ISPBufferRef>(output.getTexture());
        
        //Input indexer
        size_t Ni = real->bytesPerRow()/sizeof(float16x4_t);
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
        
        //TODO: what about the nyquist component?
        //TODO: check the loop ranges for one off on size by miscounting nyquist
        //TODO: what about alignment and row padding? I think I need to add some extra for that CVPixelBufferGetBytesPerRow can be used to correct for line padding.
        //Normalization is not needed because the underlying FFT is normalized.
        if (orientation == FSTFinalizeOrientation::Cols) {
            for (size_t i = 0; i < height; i++) {
                
                float32x4_t fre = vcvt_f32_f16( fre16[iidx(i, 0)] );
                
                float32x4_t accum = {0.0f, 0.0f, 0.0f, 0.0f};
                
                F16[oidx(i, 0)] = vcvt_f16_f32( vmulq_n_f32(fre, 0.5f) );
                F16[oidx(i, 1)] = vcvt_f16_f32(accum);
                
                for (size_t j = 0; j < N/2 - 1; j++) {
                    fre = vcvt_f32_f16( fre16[iidx(i, j)] );
                    accum = vaddq_f32(accum, fre);
                    F16[oidx(i, 2*j)] = vcvt_f16_f32( vmulq_n_f32( vcvt_f32_f16(fim16[iidx(i, j)]), k) );
                    F16[oidx(i, 2*j + 1)] = vcvt_f16_f32( vmulq_n_f32(accum, k) );
                }
            }
        } else {
            for (size_t i = 0; i < width; i++) {
                
                float32x4_t fre = vcvt_f32_f16( fre16[iidx(0, i)] );
                
                float32x4_t accum = {0.0f, 0.0f, 0.0f, 0.0f};
                
                F16[oidx(0, i)] = vcvt_f16_f32( vmulq_n_f32(fre, 0.5f) );
                F16[oidx(1, i)] = vcvt_f16_f32(accum);
                
                for (size_t j = 0; j < N/2 - 1; j++) {
                    fre = vcvt_f32_f16( fre16[iidx(j, i)] );
                    accum = vaddq_f32(accum, fre);
                    F16[oidx(2*j, i)] = vcvt_f16_f32( vmulq_n_f32( vcvt_f32_f16(fim16[iidx(j, i)]), k) );
                    F16[oidx(2*j + 1, i)] = vcvt_f16_f32( vmulq_n_f32(accum, k) );
                }
            }
        }
    };
}
std::function<void (ISSingleton&, ISComplex&)> fstFinalizeInverse_Re16RgbaNEON(GLuint width, GLuint height, FSTFinalizeOrientation orientation)
{
    //Width is the size of the target buffer, the source buffer must be half this
    //TODO: get an assertion for this assumption here
    return [=] (ISSingleton& input, ISComplex& output) {
        size_t N = width;
        
        ISPBufferRef real = dynamic_cast<ISPBufferRef>(output.getReal());
        ISPBufferRef imag = dynamic_cast<ISPBufferRef>(output.getImag());
        ISPBufferRef in = dynamic_cast<ISPBufferRef>(input.getTexture());
        //Input indexer
        size_t No = real->bytesPerRow()/sizeof(float16x4_t);
        size_t Ni = in->bytesPerRow()/sizeof(float16x4_t);
        auto oidx = [=] (size_t r, size_t c) -> size_t {
            return No*r + c;
        };
        //Output indexer
        auto iidx = [=] (size_t r, size_t c) -> size_t {
            return Ni*r + c;
        };
        float16x4_t* fre16 = (float16x4_t*)real->baseAddress();
        float16x4_t* fim16 = (float16x4_t*)imag->baseAddress();
        float16x4_t* F16 = (float16x4_t*)in->baseAddress();
        
        //TODO: what about the nyquist component?
        //TODO: check the loop ranges for one off on size by miscounting nyquist
        const float k = sqrt(2.0f/width);
        for (size_t i = 0; i < height; i++) {
            
            float32x4_t fre = vcvt_f32_f16( F16[iidx(i, 0)] );
            
            float32x4_t accum = {0.0f, 0.0f, 0.0f, 1.0f};
            
            fre16[oidx(i, 0)] = vcvt_f16_f32( vmulq_n_f32(fre, 0.5f) );
            fim16[oidx(i, 0)] = vcvt_f16_f32(accum);
            
            for (size_t j = 0; j < N/2 - 1; j++) {
                fre = vcvt_f32_f16( F16[iidx(i, 2*j)] );
                accum = vaddq_f32(accum, fre);
                fre16[oidx(i, j)] = vcvt_f16_f32( vmulq_n_f32(vcvt_f32_f16(F16[iidx(i, 2*j)]), k) );
                fim16[oidx(i, j)] = vcvt_f16_f32( vmulq_n_f32(accum, k) );
            }
        }
    };
}