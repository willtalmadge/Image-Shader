//
//  FFTTransformers.h
//  Image Shader
//
//  Created by William Talmadge on 7/19/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#ifndef __Image_Shader__FFTTransformers__
#define __Image_Shader__FFTTransformers__

#include <vector>
#include <functional>
#include "ISTexture.h"
#include "ISComplex.h"
#include "FFTSubBlock.h"
#include "FFTPermute.h"
#include "FFTRealTransformNDC.h"
#include "FFTRealTransformLowFreq.h"
#include "FFTRealTransformHighFreq.h"
#include "FFTGaussianFilter.h"
#include "FSTPrecondition.h"
#include "FSTFinalize.h"
#include "ISPipelineBufferable.h"
#include "ISRe16Rgba.h"

template<class TextureBaseT = ISTexture>
std::function<void (ISSingleton&, ISComplex&)> permuteEvenToRealAndOddToImag(GLuint width, GLuint height, FFTPermute::Orientation orientation, std::vector<GLuint> plan)
{
    return [=] (ISSingleton& input, ISComplex& output) {
        std::unique_ptr<ISSingleton> even =
        input.pipeline().transform<ISSingleton, ISRe16Rgba, FFTPermute, TextureBaseT>
        (width, height,
         FFTPermute(width, height,
                    FFTPermute::Stride::SkipOne,
                    FFTPermute::Offset::Zero,
                    orientation, plan)
         ).template result<ISSingleton>();
        std::unique_ptr<ISSingleton> odd =
        input.pipeline().transform<ISSingleton, ISRe16Rgba, FFTPermute, TextureBaseT>
        (width, height,
         FFTPermute(width, height,
                    FFTPermute::Stride::SkipOne,
                    FFTPermute::Offset::One,
                    orientation, plan)
         ).template result<ISSingleton>();
        output.setup(even, odd);
    };
}

template<class TextureBaseT = ISTexture>
std::function<void (ISComplex&, ISComplex&)> permuteComplex(GLuint width, GLuint height, FFTPermute::Orientation orientation, std::vector<GLuint> plan)
{
    return [=] (ISComplex& input, ISComplex& output) {
        std::unique_ptr<ISSingleton> real =
        input.getReal()->asSingleton()->pipeline().transform<ISSingleton, ISRe16Rgba, FFTPermute, TextureBaseT>
        (width, height,
         FFTPermute(width, height,
                    FFTPermute::Stride::SkipNone,
                    FFTPermute::Offset::Zero,
                    orientation, plan)
         ).template result<ISSingleton>();
        std::unique_ptr<ISSingleton> imag =
        input.getImag()->asSingleton()->pipeline().transform<ISSingleton, ISRe16Rgba, FFTPermute, TextureBaseT>
        (width, height,
         FFTPermute(width, height,
                    FFTPermute::Stride::SkipNone,
                    FFTPermute::Offset::Zero,
                    orientation, plan)
         ).template result<ISSingleton>();
        output.setup(real, imag);
    };
}
template<class TextureBaseT = ISTexture>
std::function<void (ISComplex&, ISComplex&)> butterflyStage(GLuint width, GLuint height, FFTSubBlock::Orientation orientation, GLuint b1, GLuint b2, GLuint blockCapacity, int sign)
{
    return [=] (ISComplex& input, ISComplex& output) {
        
        std::unique_ptr<ISSingleton> real =
        input.pipeline().multipassTransform<ISComplex, ISRe16Rgba, TextureBaseT>
        (width, height,
         [=] (ISSingleton& output, ISPipeline& pipeline){
             for (GLuint s = 0; s < b1; s++) {
                 pipeline.drawablePass<ISComplex>
                 (output,
                  FFTSubBlock(width, height, orientation, b1, b2, blockCapacity, s, FFTSubBlock::OutType::Real, sign));
             }
         }).template result<ISSingleton>();
        
        std::unique_ptr<ISSingleton> imag =
        input.pipeline().multipassTransform<ISComplex, ISRe16Rgba, TextureBaseT>
        (width, height,
         [=] (ISSingleton& output, ISPipeline& pipeline){
             for (GLuint s = 0; s < b1; s++) {
                 pipeline.drawablePass<ISComplex>
                 (output,
                  FFTSubBlock(width, height, orientation, b1, b2, blockCapacity, s, FFTSubBlock::OutType::Imag, sign));
             }
         }).template result<ISSingleton>();
        output.setup(real, imag);
    };
}
template<class TextureBaseT = ISTexture>
void butterflyAll(ISPipeline& pipeline, GLuint width, GLuint height, FFTSubBlock::Orientation orientation, int sign, std::vector<GLuint> factors)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (orientation == FFTSubBlock::Orientation::Cols) {
        FFTSubBlock::renderPhaseTable(width, sign);
    } else {
        FFTSubBlock::renderPhaseTable(height, sign);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    GLuint blockCapacity = 1;
    for (GLuint b = 0; b < factors.size() - 1; b++) {
        pipeline.transform<ISComplex, ISComplex>
        (   butterflyStage<TextureBaseT>(width, height, orientation, factors[b], factors[b+1], blockCapacity, sign));
        blockCapacity*=factors[b];
    }
    pipeline.transform<ISComplex, ISComplex>(butterflyStage<TextureBaseT>(width, height, orientation, factors.back(), 0, blockCapacity, sign));
}
template<class TextureBaseT = ISTexture>
std::function<void (ISComplex&, ISComplex&)> realTransform(GLuint width, GLuint height, int sign, ISDirection direction)
{
    return [=] (ISComplex& input, ISComplex& output) {
        std::unique_ptr<ISSingleton> real =
        input.pipeline().multipassTransform<ISComplex, ISRe16Rgba, TextureBaseT>
        (width, height,
         [=] (ISSingleton& output, ISPipeline& pipeline) {
             pipeline.drawablePass<ISComplex>(output, FFTRealTransformNDC(width, height, FFTRealTransformNDC::OutType::Real, sign, direction));
             pipeline.drawablePass<ISComplex>(output, FFTRealTransformHighFreq(width, height, FFTRealTransformHighFreq::OutType::Real, sign, direction));
             pipeline.drawablePass<ISComplex>(output, FFTRealTransformLowFreq(width, height, FFTRealTransformLowFreq::OutType::Real, sign, direction));
             
         }).template result<ISSingleton>();
        
        std::unique_ptr<ISSingleton> imag =
        input.pipeline().multipassTransform<ISComplex, ISRe16Rgba, TextureBaseT>
        (width, height,
         [=] (ISSingleton& output, ISPipeline& pipeline) {
             pipeline.drawablePass<ISComplex>(output, FFTRealTransformNDC(width, height, FFTRealTransformNDC::OutType::Imag, sign, direction));
             pipeline.drawablePass<ISComplex>(output, FFTRealTransformLowFreq(width, height, FFTRealTransformLowFreq::OutType::Imag, sign, direction));
             pipeline.drawablePass<ISComplex>(output, FFTRealTransformHighFreq(width, height, FFTRealTransformHighFreq::OutType::Imag, sign, direction));
         }).template result<ISSingleton>();
        output.setup(real, imag);
    };
}
template<class TextureBaseT>
void gaussianBlur(ISPipeline& pipeline, GLuint w, GLuint h, GLfloat sigma)
{
    std::vector<GLuint> factors = collectTwos(factorInteger(w/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(w/2, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2, w, -1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //Forward real 2d transform
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag(w/2, h, FFTPermute::Orientation::Cols, factors));
    
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Cols, 1, factors);
    
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(w/2, h, 1, ISDirection::Rows));
    
    std::vector<GLuint> factorsv = collectTwos(factorInteger(h));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(w/2, h, FFTPermute::Orientation::Rows, factorsv));
    
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Rows, 1, factorsv);
    
    //Gaussian filter
    glClearColor(0.0, 0.0, 0.0, 1.0);
    
    pipeline.transform<ISComplex, ISComplex>
    (   [=](ISComplex& input, ISComplex& output) {
        std::unique_ptr<ISSingleton> real =
        input.getReal()->asSingleton()->pipeline().transform<ISSingleton, ISRe16Rgba, FFTGaussianFilter, ISTexture>
        (w/2, h,
         FFTGaussianFilter(w/2, h, sigma)).result<ISSingleton>();
        std::unique_ptr<ISSingleton> imag =
        input.getImag()->asSingleton()->pipeline().transform<ISSingleton, ISRe16Rgba, FFTGaussianFilter, ISTexture>
        (w/2, h,
         FFTGaussianFilter(w/2, h, sigma)).result<ISSingleton>();
        output.setup(real, imag);
    });
    
    //Inverse real 2D transform
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(w/2, h, FFTPermute::Orientation::Rows, factorsv));
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Rows, -1, factorsv);
    
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(w/2, h, -1, ISDirection::Rows));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(w/2, h, FFTPermute::Orientation::Cols, factors));
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Cols, -1, factors);
}
template<class TextureBaseT>
void fftInversionTestRowMajor2D(ISPipeline& pipeline, GLuint w, GLuint h)
{
    //pass
    std::vector<GLuint> factors = collectTwos(factorInteger(w/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(w/2, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2, w, -1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //Forward real 2d transform
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag(w/2, h, FFTPermute::Orientation::Cols, factors));
    
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Cols, 1, factors);
    
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(w/2, h, 1, ISDirection::Rows));
    
    std::vector<GLuint> factorsv = collectTwos(factorInteger(h));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(w/2, h, FFTPermute::Orientation::Rows, factorsv));
    
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Rows, 1, factorsv);
    
    //Inverse real 2D transform
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(w/2, h, FFTPermute::Orientation::Rows, factorsv));
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Rows, -1, factorsv);
    
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(w/2, h, -1, ISDirection::Rows));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(w/2, h, FFTPermute::Orientation::Cols, factors));
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Cols, -1, factors);
}
template<class TextureBaseT>
void fftInversionTestColMajor2D(ISPipeline& pipeline, GLuint w, GLuint h)
{
    //pass
    std::vector<GLuint> factors = collectTwos(factorInteger(h/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(h/2, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2, h, -1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //Forward real 2d transform
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag(w, h/2, FFTPermute::Orientation::Rows, factors));
    
    butterflyAll(pipeline, w, h/2, FFTSubBlock::Orientation::Rows, 1, factors);
    
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(w, h/2, 1, ISDirection::Cols));
    
    std::vector<GLuint> factorsv = collectTwos(factorInteger(w));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(w, h/2, FFTPermute::Orientation::Cols, factorsv));
    
    butterflyAll(pipeline, w, h/2, FFTSubBlock::Orientation::Cols, 1, factorsv);
    
    //Inverse real 2D transform
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(w, h/2, FFTPermute::Orientation::Cols, factorsv));
    butterflyAll(pipeline, w, h/2, FFTSubBlock::Orientation::Cols, -1, factorsv);
    
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(w, h/2, -1, ISDirection::Cols));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(w, h/2, FFTPermute::Orientation::Rows, factors));
    butterflyAll(pipeline, w, h/2, FFTSubBlock::Orientation::Rows, -1, factors);
}
template<class TextureBaseT>
void fftInversionTestColMajor1D(ISPipeline& pipeline, GLuint w, GLuint h)
{
    //pass
    std::vector<GLuint> factors = collectTwos(factorInteger(h/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(h/2, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2, h, -1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //Forward 1d transform on columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag(w, h/2, FFTPermute::Orientation::Rows, factors));
    butterflyAll(pipeline, w, h/2, FFTSubBlock::Orientation::Rows, 1, factors);
    
    //Inverse 1d transform on columns
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(w, h/2, FFTPermute::Orientation::Rows, factors));
    butterflyAll(pipeline, w, h/2, FFTSubBlock::Orientation::Rows, -1, factors);
}
template<class TextureBaseT>
void rftInversionTestRowMajor1D(ISPipeline& pipeline, GLuint w, GLuint h)
{
    //pass
    std::vector<GLuint> factors = collectTwos(factorInteger(w/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(w/2, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2, w, -1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //Forward 1d transform on columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag(w/2, h, FFTPermute::Orientation::Cols, factors));
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Cols, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(w/2, h, 1, ISDirection::Rows));
    
    //Inverse 1d transform on columns
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(w/2, h, -1, ISDirection::Rows));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(w/2, h, FFTPermute::Orientation::Cols, factors));
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Cols, -1, factors);
}
template<class TextureBaseT>
void rftInversionTestColMajor1D(ISPipeline& pipeline, GLuint w, GLuint h)
{
    //pass
    std::vector<GLuint> factors = collectTwos(factorInteger(h/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(h/2, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2, h, -1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //Forward 1d transform on columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag(w, h/2, FFTPermute::Orientation::Rows, factors));
    butterflyAll(pipeline, w, h/2, FFTSubBlock::Orientation::Rows, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(w, h/2, 1, ISDirection::Cols));
    
    //Inverse 1d transform on columns
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(w, h/2, -1, ISDirection::Cols));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(w, h/2, FFTPermute::Orientation::Rows, factors));
    butterflyAll(pipeline, w, h/2, FFTSubBlock::Orientation::Rows, -1, factors);
}
void fstInversionTestRowMajor1D(ISPipelineBufferable& pipeline, GLuint w, GLuint h);
void fstInversionTest2D(ISPipelineBufferable& pipeline, GLuint w, GLuint h);

#endif /* defined(__Image_Shader__FFTTransformers__) */
