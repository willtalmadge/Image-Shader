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
std::function<void (ISSingleton&, ISComplex&, ISRect, ISRect)> permuteEvenToRealAndOddToImag(FFTPermute::Orientation orientation, std::vector<GLuint> plan)
{
    uint wdiv, hdiv;
    if (orientation == FFTPermute::Orientation::Cols) {
        wdiv = 2; hdiv = 1;
    } else if (orientation == FFTPermute::Orientation::Rows) {
        wdiv = 1; hdiv = 2;
    }
    return [=] (ISSingleton& input, ISComplex& output, ISRect sourceROI, ISRect targetROI) {
        std::unique_ptr<ISSingleton> even =
        //FIXME: this is using _value size to override the ROI, which ROI should I use?
        input.pipeline().fromROI(sourceROI)
                        .sourceToTargetSizeDiv(wdiv, hdiv)
                        .transform<ISSingleton, ISRe16Rgba, FFTPermute, TextureBaseT>
        (FFTPermute(FFTPermute::Stride::SkipOne,
                    FFTPermute::Offset::Zero,
                    orientation, plan)
         ).template result<ISSingleton>();
        std::unique_ptr<ISSingleton> odd =
        input.pipeline().fromROI(sourceROI)
                        .sourceToTargetSizeDiv(wdiv, hdiv)
                        .transform<ISSingleton, ISRe16Rgba, FFTPermute, TextureBaseT>
        (FFTPermute(FFTPermute::Stride::SkipOne,
                    FFTPermute::Offset::One,
                    orientation, plan)
         ).template result<ISSingleton>();
        output.setup(even, odd);
    };
}

template<class TextureBaseT = ISTexture>
std::function<void (ISComplex&, ISComplex&, ISRect, ISRect)> permuteComplex(FFTPermute::Orientation orientation, std::vector<GLuint> plan)
{
    return [=] (ISComplex& input, ISComplex& output, ISRect sourceROI, ISRect targetROI) {
        assert(sourceROI.size() == targetROI.size());
        std::unique_ptr<ISSingleton> real =
        input.getReal()->asSingleton()
             ->pipeline().fromROI(sourceROI)
                         .toROI(targetROI)
                         .transform<ISSingleton, ISRe16Rgba, FFTPermute, TextureBaseT>
        (FFTPermute(FFTPermute::Stride::SkipNone,
                    FFTPermute::Offset::Zero,
                    orientation, plan)
         ).template result<ISSingleton>();
        std::unique_ptr<ISSingleton> imag =
        input.getImag()->asSingleton()
             ->pipeline().fromROI(sourceROI)
                         .toROI(targetROI)
                         .transform<ISSingleton, ISRe16Rgba, FFTPermute, TextureBaseT>
        (FFTPermute(FFTPermute::Stride::SkipNone,
                    FFTPermute::Offset::Zero,
                    orientation, plan)
         ).template result<ISSingleton>();
        output.setup(real, imag);
    };
}
template<class TextureBaseT = ISTexture>
std::function<void (ISComplex&, ISComplex&, ISRect, ISRect)> butterflyStage(FFTSubBlock::Orientation orientation, GLuint b1, GLuint b2, GLuint blockCapacity, int sign)
{
    return [=] (ISComplex& input, ISComplex& output, ISRect sourceROI, ISRect targetROI) {
        std::unique_ptr<ISSingleton> real =
        input.pipeline().multipassTransform<ISComplex, ISRe16Rgba, TextureBaseT>
        ([=] (ISSingleton& output, ISPipeline& pipeline){
             for (GLuint s = 0; s < b1; s++) {
                 pipeline.drawablePass<ISComplex>
                 (output,
                  FFTSubBlock(orientation, b1, b2, blockCapacity, s, FFTSubBlock::OutType::Real, sign));
             }
         }).template result<ISSingleton>();
        
        std::unique_ptr<ISSingleton> imag =
        input.pipeline().multipassTransform<ISComplex, ISRe16Rgba, TextureBaseT>
        ([=] (ISSingleton& output, ISPipeline& pipeline){
             for (GLuint s = 0; s < b1; s++) {
                 pipeline.drawablePass<ISComplex>
                 (output,
                  FFTSubBlock(orientation, b1, b2, blockCapacity, s, FFTSubBlock::OutType::Imag, sign));
             }
         }).template result<ISSingleton>();
        output.setup(real, imag);
    };
}
template<class TextureBaseT = ISTexture>
void butterflyAll(ISPipeline& pipeline, FFTSubBlock::Orientation orientation, int sign, std::vector<GLuint> factors)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (orientation == FFTSubBlock::Orientation::Cols) {
        FFTSubBlock::renderPhaseTable(pipeline.targetROI().width(), sign);
    } else {
        FFTSubBlock::renderPhaseTable(pipeline.targetROI().height(), sign);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    GLuint blockCapacity = 1;
    for (GLuint b = 0; b < factors.size() - 1; b++) {
        pipeline.transform<ISComplex, ISComplex>
        (   butterflyStage<TextureBaseT>(orientation, factors[b], factors[b+1], blockCapacity, sign));
        blockCapacity*=factors[b];
    }
    pipeline.transform<ISComplex, ISComplex>(butterflyStage<TextureBaseT>(orientation, factors.back(), 0, blockCapacity, sign));
}
template<class TextureBaseT = ISTexture>
std::function<void (ISComplex&, ISComplex&, ISRect, ISRect)> realTransform(int sign, ISDirection direction)
{
    return [=] (ISComplex& input, ISComplex& output, ISRect sourceROI, ISRect targetROI) {
        std::unique_ptr<ISSingleton> real =
        input.pipeline().multipassTransform<ISComplex, ISRe16Rgba, TextureBaseT>
        ([=] (ISSingleton& output, ISPipeline& pipeline) {
             pipeline.drawablePass<ISComplex>(output, FFTRealTransformNDC(FFTRealTransformNDC::OutType::Real, sign, direction));
             pipeline.drawablePass<ISComplex>(output, FFTRealTransformHighFreq(FFTRealTransformHighFreq::OutType::Real, sign, direction));
             pipeline.drawablePass<ISComplex>(output, FFTRealTransformLowFreq(FFTRealTransformLowFreq::OutType::Real, sign, direction));
             
         }).template result<ISSingleton>();
        
        std::unique_ptr<ISSingleton> imag =
        input.pipeline().multipassTransform<ISComplex, ISRe16Rgba, TextureBaseT>
        ([=] (ISSingleton& output, ISPipeline& pipeline) {
             pipeline.drawablePass<ISComplex>(output, FFTRealTransformNDC(FFTRealTransformNDC::OutType::Imag, sign, direction));
             pipeline.drawablePass<ISComplex>(output, FFTRealTransformLowFreq(FFTRealTransformLowFreq::OutType::Imag, sign, direction));
             pipeline.drawablePass<ISComplex>(output, FFTRealTransformHighFreq(FFTRealTransformHighFreq::OutType::Imag, sign, direction));
         }).template result<ISSingleton>();
        output.setup(real, imag);
    };
}
template<class TextureBaseT>
void gaussianBlur(ISPipeline& pipeline, GLfloat sigma)
{
    uint w = pipeline.targetROI().width();
    uint h = pipeline.targetROI().height();
    std::vector<GLuint> factors = collectTwos(factorInteger(w/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(w/2, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2, w, -1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //Forward real 2d transform
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag(FFTPermute::Orientation::Cols, factors));
    
    butterflyAll(pipeline, FFTSubBlock::Orientation::Cols, 1, factors);
    
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(1, ISDirection::Rows));
    
    std::vector<GLuint> factorsv = collectTwos(factorInteger(h));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(FFTPermute::Orientation::Rows, factorsv));
    
    butterflyAll(pipeline, FFTSubBlock::Orientation::Rows, 1, factorsv);
    
    //Gaussian filter
    glClearColor(0.0, 0.0, 0.0, 1.0);
    
    pipeline.transform<ISComplex, ISComplex>
    ([=](ISComplex& input, ISComplex& output, ISRect sourceROI, ISRect targetROI) {
        std::unique_ptr<ISSingleton> real =
        input.getReal()->asSingleton()->pipeline().transform<ISSingleton, ISRe16Rgba, FFTGaussianFilter, ISTexture>
        (FFTGaussianFilter(sigma)).result<ISSingleton>();
        std::unique_ptr<ISSingleton> imag =
        input.getImag()->asSingleton()->pipeline().transform<ISSingleton, ISRe16Rgba, FFTGaussianFilter, ISTexture>
        (FFTGaussianFilter(sigma)).result<ISSingleton>();
        output.setup(real, imag);
    });
   
    //Inverse real 2D transform
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(FFTPermute::Orientation::Rows, factorsv));
    butterflyAll(pipeline, FFTSubBlock::Orientation::Rows, -1, factorsv);
    
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(-1, ISDirection::Rows));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(FFTPermute::Orientation::Cols, factors));
    butterflyAll(pipeline, FFTSubBlock::Orientation::Cols, -1, factors);
}
template<class TextureBaseT>
void fftInversionTestRowMajor2D(ISPipeline& pipeline)
{
    //pass
    uint w = pipeline.targetROI().width();
    uint h = pipeline.targetROI().height();
    std::vector<GLuint> factors = collectTwos(factorInteger(w/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(w/2, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2, w, -1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //Forward real 2d transform
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag(FFTPermute::Orientation::Cols, factors));
    
    butterflyAll(pipeline, FFTSubBlock::Orientation::Cols, 1, factors);
    
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(1, ISDirection::Rows));
    
    std::vector<GLuint> factorsv = collectTwos(factorInteger(h));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(FFTPermute::Orientation::Rows, factorsv));
    
    butterflyAll(pipeline, FFTSubBlock::Orientation::Rows, 1, factorsv);
    
    //Inverse real 2D transform
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(FFTPermute::Orientation::Rows, factorsv));
    butterflyAll(pipeline, FFTSubBlock::Orientation::Rows, -1, factorsv);
    
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(-1, ISDirection::Rows));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(FFTPermute::Orientation::Cols, factors));
    butterflyAll(pipeline, FFTSubBlock::Orientation::Cols, -1, factors);
}
template<class TextureBaseT>
void fftInversionTestColMajor2D(ISPipeline& pipeline)
{
    //pass
    uint w = pipeline.targetROI().width();
    uint h = pipeline.targetROI().height();
    std::vector<GLuint> factors = collectTwos(factorInteger(h/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(h/2, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2, h, -1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //Forward real 2d transform
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag(FFTPermute::Orientation::Rows, factors));
    
    butterflyAll(pipeline, FFTSubBlock::Orientation::Rows, 1, factors);
    
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(1, ISDirection::Cols));
    
    std::vector<GLuint> factorsv = collectTwos(factorInteger(w));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(FFTPermute::Orientation::Cols, factorsv));
    
    butterflyAll(pipeline, FFTSubBlock::Orientation::Cols, 1, factorsv);
    
    //Inverse real 2D transform
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(FFTPermute::Orientation::Cols, factorsv));
    butterflyAll(pipeline, FFTSubBlock::Orientation::Cols, -1, factorsv);
    
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(-1, ISDirection::Cols));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(FFTPermute::Orientation::Rows, factors));
    butterflyAll(pipeline, FFTSubBlock::Orientation::Rows, -1, factors);
}
template<class TextureBaseT>
void fftInversionTestColMajor1D(ISPipeline& pipeline)
{
    //pass
    uint h = pipeline.targetROI().height();
    std::vector<GLuint> factors = collectTwos(factorInteger(h/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(h/2, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2, h, -1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //Forward 1d transform on columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag(FFTPermute::Orientation::Rows, factors));
    butterflyAll(pipeline, FFTSubBlock::Orientation::Rows, 1, factors);

    //Inverse 1d transform on columns
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(FFTPermute::Orientation::Rows, factors));
    butterflyAll(pipeline, FFTSubBlock::Orientation::Rows, -1, factors);
}
template<class TextureBaseT>
void fftInversionTestRowMajor1D(ISPipeline& pipeline)
{
    //pass
    uint w = pipeline.targetROI().width();
    std::vector<GLuint> factors = collectTwos(factorInteger(w/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(w/2, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2, w, -1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //Forward 1d transform on columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag(FFTPermute::Orientation::Cols, factors));
    butterflyAll(pipeline, FFTSubBlock::Orientation::Cols, 1, factors);
    
    //Inverse 1d transform on columns
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(FFTPermute::Orientation::Cols, factors));
    butterflyAll(pipeline, FFTSubBlock::Orientation::Cols, -1, factors);
}
template<class TextureBaseT>
void rftInversionTestRowMajor1D(ISPipeline& pipeline)
{
    //pass
    uint w = pipeline.targetROI().width();
    std::vector<GLuint> factors = collectTwos(factorInteger(w/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(w/2, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2, w, -1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //Forward 1d transform on columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag(FFTPermute::Orientation::Cols, factors));
    butterflyAll(pipeline, FFTSubBlock::Orientation::Cols, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(1, ISDirection::Rows));
    
    //Inverse 1d transform on columns
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(-1, ISDirection::Rows));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(FFTPermute::Orientation::Cols, factors));
    butterflyAll(pipeline, FFTSubBlock::Orientation::Cols, -1, factors);
}
template<class TextureBaseT>
void rftInversionTestColMajor1D(ISPipeline& pipeline)
{
    //pass
    uint h = pipeline.targetROI().height();
    std::vector<GLuint> factors = collectTwos(factorInteger(h/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(h/2, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2, h, -1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //Forward 1d transform on columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag(FFTPermute::Orientation::Rows, factors));
    butterflyAll(pipeline, FFTSubBlock::Orientation::Rows, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(1, ISDirection::Cols));
    
    //Inverse 1d transform on columns
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform(-1, ISDirection::Cols));
    pipeline.transform<ISComplex, ISComplex>
    (   permuteComplex(FFTPermute::Orientation::Rows, factors));
    butterflyAll(pipeline, FFTSubBlock::Orientation::Rows, -1, factors);
}
void fstInversionTestRowMajor1D(ISPipelineBufferable& pipeline);
void fstInversionTestColMajor1D(ISPipelineBufferable& pipeline);
void fstInversionTest2D(ISPipelineBufferable& pipeline, ISRect roi);
void poisson2D(ISPipelineBufferable& pipeline, ISRect roi);
std::function<void (ISComplex&, ISSingleton&, ISRect, ISRect)> selectRealDiscardComplexDoubleRows();

#endif /* defined(__Image_Shader__FFTTransformers__) */
