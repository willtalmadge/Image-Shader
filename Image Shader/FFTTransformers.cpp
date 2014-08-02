//
//  FFTTransformers.cpp
//  Image Shader
//
//  Created by William Talmadge on 7/19/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#include "FFTTransformers.h"
#include "FSTLaplace.h"
#include "ISPassThroughDrawable.h"
#include "ISURe8Rgba.h"

using namespace std;

void fstInversionTestRowMajor1D(ISPipelineBufferable& pipeline)
{
    //PASS
    uint w = pipeline.targetROI().width();
    std::vector<GLuint> factors = collectTwos(factorInteger(w/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(w/2, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2, w, -1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, -1);
    FFTPhaseTable::renderPhaseTable(w, 2*w, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);

    //FST precondition
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //FIXME: This is really important, it doesn't work at all without this. Need to set the first pixel column to black more robustly.
    pipeline.transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
    (FSTPrecondition(FSTPrecondition::Orientation::Cols));

    //Forward real transform across columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(FFTPermute::Orientation::Cols, factors));
    butterflyAll<ISPBuffer>(pipeline, FFTSubBlock::Orientation::Cols, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(1, ISDirection::Rows));

    //FST finalize
    pipeline.sourceToTargetSizeMult(2, 1).transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
    (fstFinalize_Re16RgbaVFP4(FSTFinalizeOrientation::Cols));

    //Do the transform again to invert
    //FST precondition
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //FIXME: This is really important, it doesn't work at all without this. Need to set the first pixel column to black more robustly.
    pipeline.transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
    (FSTPrecondition(FSTPrecondition::Orientation::Cols));

    //Forward real transform across columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(FFTPermute::Orientation::Cols, factors));
/*    butterflyAll<ISPBuffer>(pipeline, FFTSubBlock::Orientation::Cols, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(1, ISDirection::Rows));

    //FST finalize
    pipeline.sourceToTargetSizeMult(2, 1).transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
    (fstFinalize_Re16RgbaVFP4(FSTFinalizeOrientation::Cols));
*/
}
void fstInversionTestColMajor1D(ISPipelineBufferable& pipeline)
{
    //PASS
    uint h = pipeline.targetROI().height();
    std::vector<GLuint> factors = collectTwos(factorInteger(h/2));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(h/2, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2, h, -1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, 1);
    FFTPhaseTable::renderPhaseTable(h/2+1, h, -1);
    FFTPhaseTable::renderPhaseTable(h, 2*h, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //FST precondition
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //FIXME: This is really important, it doesn't work at all without this. Need to set the first pixel column to black more robustly.
    pipeline.transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
    (FSTPrecondition(FSTPrecondition::Orientation::Rows));

    //Forward real transform across columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(FFTPermute::Orientation::Rows, factors));
    butterflyAll<ISPBuffer>(pipeline, FFTSubBlock::Orientation::Rows, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(1, ISDirection::Cols));
    
    //FST finalize
    pipeline.sourceToTargetSizeMult(1, 2).transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
    (fstFinalize_Re16RgbaVFP4(FSTFinalizeOrientation::Rows));
    
    //Do the transform again to invert
    //FST precondition
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //FIXME: This is really important, it doesn't work at all without this. Need to set the first pixel column to black more robustly.
    pipeline.transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
    (FSTPrecondition(FSTPrecondition::Orientation::Rows));
    
    //Forward real transform across columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(FFTPermute::Orientation::Rows, factors));
    butterflyAll<ISPBuffer>(pipeline, FFTSubBlock::Orientation::Rows, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(1, ISDirection::Cols));
    
    //FST finalize
    pipeline.sourceToTargetSizeMult(1, 2).transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
    (fstFinalize_Re16RgbaVFP4(FSTFinalizeOrientation::Rows));
}
void fstInversionTest2D(ISPipelineBufferable& pipeline, ISRect roi)
{
    //PASS
    uint w = roi.width();
    uint h = roi.height();
    std::vector<GLuint> factors = collectTwos(factorInteger(w/2));
    std::vector<GLuint> factorsv = collectTwos(factorInteger(h/2));
    
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

    ISRect originalROI = pipeline.targetROI();
    //
    //Forward 2D transform
    //
    
    /////
    //Transform the rows
    /////

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    pipeline.setTargetSize(originalROI.size())
            .fromROI(roi)
            .toROI(roi)
            .transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
            (FSTPrecondition(FSTPrecondition::Orientation::Cols));

    //Forward real transform across columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(FFTPermute::Orientation::Cols, factors));
    butterflyAll<ISPBuffer>(pipeline, FFTSubBlock::Orientation::Cols, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(1, ISDirection::Rows));
    
    //FST finalize
    pipeline.setTargetSize(originalROI.size())
            .fromROI(ISRect(roi).right(roi.right()/2))
            .toROI(roi)
            .transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
            (fstFinalize_Re16RgbaVFP4(FSTFinalizeOrientation::Cols));

    /////
    //Transform the cols
    /////
    //FST precondition
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    pipeline.setTargetSize(originalROI.size())
            .fromROI(roi)
            .toROI(roi)
            .transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
            (FSTPrecondition(FSTPrecondition::Orientation::Rows));
  
    //Forward real transform across rows
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(FFTPermute::Orientation::Rows, factorsv));
    butterflyAll<ISPBuffer>(pipeline, FFTSubBlock::Orientation::Rows, 1, factorsv);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(1, ISDirection::Cols));
    
    //FST finalize
    pipeline.setTargetSize(originalROI.size())
            .fromROI(ISRect(roi).bottom(roi.bottom()/2))
            .toROI(roi)
            .transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
            (fstFinalize_Re16RgbaVFP4(FSTFinalizeOrientation::Rows));
  
    //
    //Inverse 2D transform
    //

    /////
    //Transform the cols
    /////
    //FST precondition
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    pipeline.setTargetSize(originalROI.size())
            .fromROI(roi)
            .toROI(roi)
            .transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
            (FSTPrecondition(FSTPrecondition::Orientation::Rows));
    
    //Forward real transform across columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(FFTPermute::Orientation::Rows, factorsv));
    butterflyAll<ISPBuffer>(pipeline, FFTSubBlock::Orientation::Rows, 1, factorsv);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(1, ISDirection::Cols));
    
    //FST finalize
    pipeline.setTargetSize(originalROI.size())
            .fromROI(ISRect(roi).bottom(roi.bottom()/2))
            .toROI(roi)
            .transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
            (fstFinalize_Re16RgbaVFP4(FSTFinalizeOrientation::Rows));

    /////
    //Transform the rows
    /////
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    pipeline.setTargetSize(originalROI.size())
            .fromROI(roi)
            .toROI(roi)
            .transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
            (FSTPrecondition(FSTPrecondition::Orientation::Cols));
    
    //Forward real transform across columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(FFTPermute::Orientation::Cols, factors));
    butterflyAll<ISPBuffer>(pipeline, FFTSubBlock::Orientation::Cols, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(1, ISDirection::Rows));
    
    //FST finalize
    pipeline.setTargetSize(originalROI.size())
            .fromROI(ISRect(roi).right(roi.right()/2))
            .toROI(roi)
            .transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
            (fstFinalize_Re16RgbaVFP4(FSTFinalizeOrientation::Cols));
}
void poisson2D(ISPipelineBufferable& pipeline, ISRect roi)
{
    //PASS
    uint w = roi.width();
    uint h = roi.height();
    std::vector<GLuint> factors = collectTwos(factorInteger(w/2));
    std::vector<GLuint> factorsv = collectTwos(factorInteger(h/2));
    
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
    
    ISRect originalROI = pipeline.targetROI();
    //
    //Forward 2D transform
    //
    
    /////
    //Transform the rows
    /////
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    pipeline.setTargetSize(originalROI.size())
    .fromROI(roi)
    .toROI(roi)
    .transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
    (FSTPrecondition(FSTPrecondition::Orientation::Cols));
    
    //Forward real transform across columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(FFTPermute::Orientation::Cols, factors));
    butterflyAll<ISPBuffer>(pipeline, FFTSubBlock::Orientation::Cols, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(1, ISDirection::Rows));
    
    //FST finalize
    pipeline.setTargetSize(originalROI.size())
    .fromROI(ISRect(roi).right(roi.right()/2))
    .toROI(roi)
    .transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
    (fstFinalize_Re16RgbaVFP4(FSTFinalizeOrientation::Cols));
    
    /////
    //Transform the cols
    /////
    //FST precondition
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    pipeline.setTargetSize(originalROI.size())
    .fromROI(roi)
    .toROI(roi)
    .transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
    (FSTPrecondition(FSTPrecondition::Orientation::Rows));
    
    //Forward real transform across rows
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(FFTPermute::Orientation::Rows, factorsv));
    butterflyAll<ISPBuffer>(pipeline, FFTSubBlock::Orientation::Rows, 1, factorsv);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(1, ISDirection::Cols));
    
    //FST finalize
    pipeline.setTargetSize(originalROI.size())
    .fromROI(ISRect(roi).bottom(roi.bottom()/2))
    .toROI(roi)
    .transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
    (fstFinalize_Re16RgbaVFP4(FSTFinalizeOrientation::Rows));
    
    pipeline.setTargetSize(originalROI.size())
    .fromROI(originalROI)
    .toROI(originalROI)
    .transform<ISSingleton, ISRe16Rgba, FSTLaplace, ISPBuffer>(FSTLaplace());
    //
    //Inverse 2D transform
    //
    
    /////
    //Transform the cols
    /////
    //FST precondition
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    pipeline.setTargetSize(originalROI.size())
    .fromROI(roi)
    .toROI(roi)
    .transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
    (FSTPrecondition(FSTPrecondition::Orientation::Rows));
    
    //Forward real transform across columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(FFTPermute::Orientation::Rows, factorsv));
    butterflyAll<ISPBuffer>(pipeline, FFTSubBlock::Orientation::Rows, 1, factorsv);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(1, ISDirection::Cols));
    
    //FST finalize
    pipeline.setTargetSize(originalROI.size())
    .fromROI(ISRect(roi).bottom(roi.bottom()/2))
    .toROI(roi)
    .transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
    (fstFinalize_Re16RgbaVFP4(FSTFinalizeOrientation::Rows));
    
    /////
    //Transform the rows
    /////
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    pipeline.setTargetSize(originalROI.size())
    .fromROI(roi)
    .toROI(roi)
    .transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
    (FSTPrecondition(FSTPrecondition::Orientation::Cols));
    
    //Forward real transform across columns
    pipeline.transform<ISSingleton, ISComplex>
    (   permuteEvenToRealAndOddToImag<ISPBuffer>(FFTPermute::Orientation::Cols, factors));
    butterflyAll<ISPBuffer>(pipeline, FFTSubBlock::Orientation::Cols, 1, factors);
    pipeline.transform<ISComplex, ISComplex>
    (   realTransform<ISPBuffer>(1, ISDirection::Rows));
    
    //FST finalize
    pipeline.setTargetSize(originalROI.size())
    .fromROI(ISRect(roi).right(roi.right()/2))
    .toROI(roi)
    .transformBuffer<ISComplex, ISSingleton, ISRe16Rgba>
    (fstFinalize_Re16RgbaVFP4(FSTFinalizeOrientation::Cols));
    
}
std::function<void (ISComplex&, ISSingleton&, ISRect, ISRect)> selectRealDiscardComplexDoubleRows() {
    return [] (ISComplex& input, ISSingleton& output, ISRect sourceROI, ISRect targetROI) {
    unique_ptr<ISSingleton> real =
    input.getImag()->asSingleton()->pipeline().sourceToTargetSizeMult(2, 1).transform<ISSingleton, ISURe8Rgba, ISPassThroughDrawable, ISTexture>
    (ISPassThroughDrawable(1.0f)).result<ISSingleton>();
    output.setup(real);
    };
}