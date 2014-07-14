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

#import "ViewController.h"
#import <GLKit/GLKit.h>
#import <CoreGraphics/CoreGraphics.h>
#import <vector>
#import "ISPassThroughDrawable.h"
#import "FFTPermute.h"
#import "FFTSubBlock.h"
#import "ISPipelineBufferable.h"
#import "ISComplex.h"
#import "FFTRealTransformNDC.h"
#import "FFTRealTransformLowFreq.h"
#import "FFTRealTransformHighFreq.h"
#import "FFTGaussianFilter.h"
#import "ISURe8Rgba.h"
#import "ISRe16Rgba.h"
#import "half.hpp"
#import <arm_neon.h>
using namespace std;
using namespace half_float;
@interface ViewController ()
{

}
@property (weak, nonatomic) IBOutlet UIImageView *imageView;

@end

@implementation ViewController

void ProviderReleaseData ( void *info, const void *data, size_t size )
{
    free((void*)data);
}

template<class TextureBaseT = ISTexture>
function<void (ISSingleton&, ISComplex&)> permuteEvenToRealAndOddToImag(GLuint width, GLuint height, FFTPermute::Orientation orientation, vector<GLuint> plan)
{
    return [=] (ISSingleton& input, ISComplex& output) {
        unique_ptr<ISSingleton> even =
        input.pipeline().transform<ISSingleton, ISRe16Rgba, FFTPermute, TextureBaseT>
        (width, height,
         FFTPermute(width, height,
                    FFTPermute::Stride::SkipOne,
                    FFTPermute::Offset::Zero,
                    orientation, plan)
         ).template result<ISSingleton>();
        unique_ptr<ISSingleton> odd =
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
function<void (ISComplex&, ISComplex&)> permuteComplex(GLuint width, GLuint height, FFTPermute::Orientation orientation, vector<GLuint> plan)
{
    return [=] (ISComplex& input, ISComplex& output) {
        unique_ptr<ISSingleton> real =
        input.getReal()->asSingleton()->pipeline().transform<ISSingleton, ISRe16Rgba, FFTPermute, TextureBaseT>
        (width, height,
         FFTPermute(width, height,
                    FFTPermute::Stride::SkipNone,
                    FFTPermute::Offset::Zero,
                    orientation, plan)
         ).template result<ISSingleton>();
        unique_ptr<ISSingleton> imag =
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
function<void (ISComplex&, ISComplex&)> butterflyStage(GLuint width, GLuint height, FFTSubBlock::Orientation orientation, GLuint b1, GLuint b2, GLuint blockCapacity, int sign)
{
    return [=] (ISComplex& input, ISComplex& output) {
        
        unique_ptr<ISSingleton> real =
        input.pipeline().multipassTransform<ISComplex, ISRe16Rgba, TextureBaseT>
        (width, height,
         [=] (ISSingleton& output, ISPipeline& pipeline){
             for (GLuint s = 0; s < b1; s++) {
                 pipeline.drawablePass<ISComplex>(output, FFTSubBlock(width, height, orientation, b1, b2, blockCapacity, s, FFTSubBlock::OutType::Real, sign));
             }
         }).template result<ISSingleton>();
        
        unique_ptr<ISSingleton> imag =
        input.pipeline().multipassTransform<ISComplex, ISRe16Rgba, TextureBaseT>
        (width, height,
         [=] (ISSingleton& output, ISPipeline& pipeline){
             for (GLuint s = 0; s < b1; s++) {
                 pipeline.drawablePass<ISComplex>(output, FFTSubBlock(width, height, orientation, b1, b2, blockCapacity, s, FFTSubBlock::OutType::Imag, sign));
             }
         }).template result<ISSingleton>();
        output.setup(real, imag);
    };
}
template<class TextureBaseT = ISTexture>
void butterflyAll(ISPipeline& pipeline, GLuint width, GLuint height, FFTSubBlock::Orientation orientation, int sign, vector<GLuint> factors)
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
        pipeline.transform<ISComplex, ISComplex>(butterflyStage<TextureBaseT>(width, height, orientation, factors[b], factors[b+1], blockCapacity, sign));
        blockCapacity*=factors[b];
    }
    pipeline.transform<ISComplex, ISComplex>(butterflyStage<TextureBaseT>(width, height, orientation, factors.back(), 0, blockCapacity, sign));
}
template<class TextureBaseT = ISTexture>
function<void (ISComplex&, ISComplex&)> realTransform(GLuint width, GLuint height, int direction)
{
    return [=] (ISComplex& input, ISComplex& output) {
        unique_ptr<ISSingleton> real =
        input.pipeline().multipassTransform<ISComplex, ISRe16Rgba, TextureBaseT>
        (width, height,
         [=] (ISSingleton& output, ISPipeline& pipeline) {
            pipeline.drawablePass<ISComplex>(output, FFTRealTransformNDC(width, height, FFTRealTransformNDC::OutType::Real, direction));
            pipeline.drawablePass<ISComplex>(output, FFTRealTransformHighFreq(width, height, FFTRealTransformHighFreq::OutType::Real, direction));
            pipeline.drawablePass<ISComplex>(output, FFTRealTransformLowFreq(width, height, FFTRealTransformLowFreq::OutType::Real, direction));

        }).template result<ISSingleton>();
        
        unique_ptr<ISSingleton> imag =
        input.pipeline().multipassTransform<ISComplex, ISRe16Rgba, TextureBaseT>
        (width, height,
         [=] (ISSingleton& output, ISPipeline& pipeline) {
            pipeline.drawablePass<ISComplex>(output, FFTRealTransformNDC(width, height, FFTRealTransformNDC::OutType::Imag, direction));
            pipeline.drawablePass<ISComplex>(output, FFTRealTransformLowFreq(width, height, FFTRealTransformLowFreq::OutType::Imag, direction));
            pipeline.drawablePass<ISComplex>(output, FFTRealTransformHighFreq(width, height, FFTRealTransformHighFreq::OutType::Imag, direction));
        }).template result<ISSingleton>();
        output.setup(real, imag);
    };
}
void readFBRow(GLuint width, GLuint y0)
{
    GLfloat *r = new GLfloat[4*width];
    glReadPixels(0, y0, width, 1, GL_RGBA, GL_FLOAT, r);
    for (int i = 0; i < width; i++) {
        cout << "{";
        for (int j = 0; j < 4; j++) {
            cout << r[4*i + j];
            if (j < 3) {
                cout << ", ";
            }
        }
        cout << "}";
        if (i < width - 1) {
            cout << ", ";
        }
        cout << endl;
    }
    delete [] r;
}
- (IBAction)applyFilter:(id)sender
{
    
    EAGLContext* context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:context];
    
    UIImage* image = _imageView.image;
    NSError* err;
    NSDictionary *options = @{GLKTextureLoaderOriginBottomLeft:@NO}; //Image data not flipped.
    GLKTextureInfo* textureInfo;
    textureInfo = [GLKTextureLoader textureWithCGImage:image.CGImage options:options error:&err];
    NSLog(@"Loaded image into %d", textureInfo.name);
    int w = image.size.width;
    int h = image.size.height;
    
    auto scale = [=] (ISComplex& input, ISComplex& output) {
        unique_ptr<ISSingleton> real =
        input.getReal()->asSingleton()->pipeline().transform<ISSingleton, ISRe16Rgba, ISPassThroughDrawable, ISTexture>
        (w/2, h,
         ISPassThroughDrawable(w/2, h, 2.0/w)).result<ISSingleton>();
        
        unique_ptr<ISSingleton> imag =
        input.getImag()->asSingleton()->pipeline().transform<ISSingleton, ISRe16Rgba, ISPassThroughDrawable, ISTexture>
        (w/2, h,
         ISPassThroughDrawable(w/2, h, 2.0/w)).result<ISSingleton>();
        output.setup(real, imag);
    };
    
    vector<GLuint> factors = collectTwos(factorInteger(w/2));
    for (int num : factors) {
        cout << num << ", ";
    }
    ISTextureRef tex = ISURe8Rgba<>::fromExisting(textureInfo.name, w, h, GL_UNSIGNED_BYTE);
    ISPipelineBufferable pipeline(unique_ptr<ISSingleton>(tex->asSingleton()));
    pipeline.setContext(context);
    pipeline.setupRoot();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(w/2, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2, w, -1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);
    
    //Forward real 2d transform
    pipeline.transform<ISSingleton, ISComplex>(permuteEvenToRealAndOddToImag(w/2, h, FFTPermute::Orientation::Cols, factors));
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Cols, 1, factors);
    pipeline.transform<ISComplex, ISComplex>(realTransform<ISPBuffer>(w/2, h, 1));
    
    struct pix16 {
        half r;
        half g;
        half b;
        half a;
    };
    pipeline.transformBuffer<ISComplex, ISRe16Rgba>
    (w/2, h,
     [](ISComplex& input, ISSingleton& output){
         const ISPBuffer* real = dynamic_cast<const ISPBuffer*>(input.getReal());
         const ISPBuffer* imag = dynamic_cast<const ISPBuffer*>(input.getImag());
         pix16* pix = (pix16*)real->baseAddress();
         float16x4_t* pixels=(float16x4_t*)real->baseAddress();
         float32x4_t v32 = vcvt_f32_f16(pixels[0]);
         printf("r%f, g%f, b%f, a%f\n", v32[0], v32[1], v32[2], v32[3]);
         printf("r%f, g%f, b%f, a%f\n", (float)pix->r, (float)pix->g, (float)pix->b, (float)pix->a);
     });
    vector<GLuint> factorsv = collectTwos(factorInteger(h));
    pipeline.transform<ISComplex, ISComplex>(permuteComplex(w/2, h, FFTPermute::Orientation::Rows, factorsv));
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Rows, 1, factorsv);
    
    //Gaussian filter
    glClearColor(0.0, 0.0, 0.0, 1.0);
    pipeline.transform<ISComplex, ISComplex>
    ([=](ISComplex& input, ISComplex& output) {
        unique_ptr<ISSingleton> real =
        input.getReal()->asSingleton()->pipeline().transform<ISSingleton, ISRe16Rgba, FFTGaussianFilter, ISTexture>
        (w/2, h,
         FFTGaussianFilter(w/2, h, 50)).result<ISSingleton>();
        unique_ptr<ISSingleton> imag =
        input.getImag()->asSingleton()->pipeline().transform<ISSingleton, ISRe16Rgba, FFTGaussianFilter, ISTexture>
        (w/2, h,
         FFTGaussianFilter(w/2, h, 50)).result<ISSingleton>();
        output.setup(real, imag);
    });
    
    //Inverse real 2D transform
    pipeline.transform<ISComplex, ISComplex>(permuteComplex(w/2, h, FFTPermute::Orientation::Rows, factorsv));
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Rows, -1, factorsv);
    pipeline.transform<ISComplex, ISComplex>(realTransform(w/2, h, -1));
    pipeline.transform<ISComplex, ISComplex>(permuteComplex(w/2, h, FFTPermute::Orientation::Cols, factors));
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Cols, -1, factors);
    
    //Read out real channel result
    auto selectRealDiscardComplex = [=] (ISComplex& input, ISSingleton& output) {
        unique_ptr<ISSingleton> real =
        input.getReal()->asSingleton()->pipeline().transform<ISSingleton, ISURe8Rgba, ISPassThroughDrawable, ISTexture>
        (w, h,
         ISPassThroughDrawable(w, h, 1.0f)).result<ISSingleton>();
        output.setup(real);
    };
    pipeline.transform<ISComplex, ISSingleton>(selectRealDiscardComplex);
    glFinish();
    
    GLint readType;
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &readType);
    NSLog(@"Supported read type %d (expecting %d)", readType, GL_UNSIGNED_BYTE);
    GLubyte *tempImagebuffer = (GLubyte *) malloc(w*h*4);
    glReadPixels( 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, tempImagebuffer);
    
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, tempImagebuffer, w*h*4, ProviderReleaseData);
    
    int bitsPerComponent = 8;
    int bitsPerPixel = 32;
    int bytesPerRow = 4*w;
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    
    CGImageRef imageRef = CGImageCreate(w, h, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);
    
    UIImage *myImage =  [UIImage imageWithCGImage:imageRef] ;
    _imageView.image = myImage;
    CGDataProviderRelease(provider);
    CGImageRelease(imageRef);
    CGColorSpaceRelease(colorSpaceRef);
    //[UIImagePNGRepresentation(myImage) writeToFile:@"/Users/williamtalmadge/Downloads/ISFFT.png" atomically:YES];
    
    pipeline.releaseAllCaches();
    pipeline.teardown();
}
- (void)viewDidLoad
{
    [super viewDidLoad];

    NSString* path = [[NSBundle mainBundle] pathForResource:@"andreewallin_small" ofType:@"png"];
    UIImage* image = [UIImage imageWithContentsOfFile:path];
    _imageView.image = image;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
