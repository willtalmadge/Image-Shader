//
//  FFTTransformers.cpp
//  Image Shader
//
//  Created by William Talmadge on 7/19/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#include "FFTTransformers.h"
void fstInversionTestRowMajor1D(ISPipelineBufferable& pipeline, GLuint w, GLuint h)
{
    std::vector<GLuint> factors = collectTwos(factorInteger(w/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(w/2, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2, w, -1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, -1);
    FFTPhaseTable::renderPhaseTable(w, 2*w, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);

    //FST precondition
    pipeline.transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
    (w, h,
     FSTPrecondition(w, h, FSTPrecondition::Orientation::Cols));

    //Forward real transform across columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(w/2, h, FFTPermute::Orientation::Cols, factors));
    butterflyAll<ISPBuffer>(pipeline, w/2, h, FFTSubBlock::Orientation::Cols, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(w/2, h, 1, ISDirection::Rows));

    //FST finalize
    pipeline.transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
    (w, h,
     fstFinalizeForward_Re16RgbaVFP4(w, h, FSTFinalizeOrientation::Cols));

    //Do the transform again to invert
    //FST precondition
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    pipeline.transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
    (w, h,
     FSTPrecondition(w, h, FSTPrecondition::Orientation::Cols));

    //Forward real transform across columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(w/2, h, FFTPermute::Orientation::Cols, factors));
    butterflyAll<ISPBuffer>(pipeline, w/2, h, FFTSubBlock::Orientation::Cols, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(w/2, h, 1, ISDirection::Rows));
    
    //FST finalize
    pipeline.transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
    (w, h,
     fstFinalizeForward_Re16RgbaVFP4(w, h, FSTFinalizeOrientation::Cols));

}
void fstInversionTest2D(ISPipelineBufferable& pipeline, GLuint w, GLuint h)
{
    std::vector<GLuint> factors = collectTwos(factorInteger(w/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(w/2, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2, w, -1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, -1);
    FFTPhaseTable::renderPhaseTable(w, 2*w, 1);
    
    FFTPhaseTable::renderPhaseTable(h/2, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2, h, -1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, -1);
    FFTPhaseTable::renderPhaseTable(h, 2*h, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //FST precondition
    pipeline.transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
    (w, h,
     FSTPrecondition(w, h, FSTPrecondition::Orientation::Cols));
    
    //Forward real transform across columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(w/2, h, FFTPermute::Orientation::Cols, factors));
    butterflyAll<ISPBuffer>(pipeline, w/2, h, FFTSubBlock::Orientation::Cols, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(w/2, h, 1, ISDirection::Rows));
    
    //FST finalize
    pipeline.transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
    (w, h,
     fstFinalizeForward_Re16RgbaVFP4(w, h, FSTFinalizeOrientation::Cols));
    
    //FST precondition
    pipeline.transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
    (w, h,
     FSTPrecondition(w, h, FSTPrecondition::Orientation::Rows));
    
    //Forward real 2D transform
    //FST is its own inverse, so both FFTs need to have the same direction.
    factors = collectTwos(factorInteger(h/2));
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(w, h/2, FFTPermute::Orientation::Rows, factors));
    butterflyAll<ISPBuffer>(pipeline, w, h/2, FFTSubBlock::Orientation::Rows, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(w, h/2, 1, ISDirection::Cols));
    
    //FST inverse finalize
    pipeline.transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
    (w, h,
     fstFinalizeForward_Re16RgbaVFP4(w, h, FSTFinalizeOrientation::Cols));
    
}