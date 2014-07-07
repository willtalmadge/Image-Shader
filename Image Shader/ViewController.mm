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
#import "ISPipeline.h"
#import "ISComplex.h"
#import "FFTRealTransformNDC.h"
#import "FFTRealTransformLowFreq.h"
#import "FFTRealTransformHighFreq.h"
#import "FFTGaussianFilter.h"

using namespace std;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
/*
GLfloat gQuadData[36] =
{
    // Data layout for each line below is:
    // positionX, positionY, positionZ,  texS, texT, colorMultiplier
    -0.5f, -0.5f, 0.5f, 0.0, 0.0,   1.0,
    0.5f, -0.5f, 0.5f,  1.0, 0.0,   0.5,
    -0.5f,  0.5f, 0.5f, 0.0, 1.0,   0.5,
    
    0.5f,  0.5f, 0.5f,  1.0, 1.0,   0.5,
    -0.5f, 0.5f, 0.5f,  0.0, 1.0,   0.5,
    0.5f, -0.5f, 0.5f,  1.0, 0.0,   0.5
};
*/

GLfloat gQuadData[36] =
{
    // Data layout for each line below is:
    // positionX, positionY, positionZ,  texS, texT, colorMultiplier
    0.0,  0.0,   0.5f,  0.0, 0.0,   1.0,
    256,  0.0,   0.5f,  1.0, 0.0,   0.5,
    0.0,  256,   0.5f,  0.0, 1.0,   0.5,
    
    256,  256,   0.5f,  1.0, 1.0,   0.5,
    0.0,  256,   0.5f,  0.0, 1.0,   0.5,
    256,  0.0,   0.5f,  1.0, 0.0,   0.5
};

//This attribute is to define a shader attribute that gets multiplied by the
//color to darken the image.
//const GLuint AttributeMultiplier = 5;

@interface ViewController ()
{
    GLuint _program;
    GLuint _vertexArray;
    GLuint _vertexBuffer;
}
@property (weak, nonatomic) IBOutlet UIImageView *imageView;

@end

@implementation ViewController

void ProviderReleaseData ( void *info, const void *data, size_t size )
{
    free((void*)data);
}

function<void (ISSingleton&, ISComplex&)> permuteEvenToRealAndOddToImag(GLuint width, GLuint height, FFTPermute::Orientation orientation, vector<GLuint> plan)
{
    return [=] (ISSingleton& input, ISComplex& output) {
        unique_ptr<ISSingleton> even =
        input.pipeline().transform<ISSingleton, ISSingleton, FFTPermute>
        ([=] (ISSingleton& input, ISSingleton& output) {
            output.setup<ISRe16Rgba>(width, height);
        }, FFTPermute(width, height,
                      FFTPermute::Stride::SkipOne,
                      FFTPermute::Offset::Zero,
                      orientation, plan)
         ).result<ISSingleton>();
        unique_ptr<ISSingleton> odd =
        input.pipeline().transform<ISSingleton, ISSingleton, FFTPermute>
        ([=] (ISSingleton& input, ISSingleton& output) {
            output.setup<ISRe16Rgba>(width, height);
        }, FFTPermute(width, height,
                      FFTPermute::Stride::SkipOne,
                      FFTPermute::Offset::One,
                      orientation, plan)
         ).result<ISSingleton>();
        output.setup(even, odd);
    };
}

function<void (ISComplex&, ISComplex&)> permuteComplex(GLuint width, GLuint height, FFTPermute::Orientation orientation, vector<GLuint> plan)
{
    return [=] (ISComplex& input, ISComplex& output) {
        unique_ptr<ISSingleton> real =
        input.getReal()->asSingleton()->pipeline().transform<ISSingleton, ISSingleton, FFTPermute>
        ([=] (ISSingleton& input, ISSingleton& output) {
            output.setup<ISRe16Rgba>(width, height);
        }, FFTPermute(width, height,
                      FFTPermute::Stride::SkipNone,
                      FFTPermute::Offset::Zero,
                      orientation, plan)
         ).result<ISSingleton>();
        unique_ptr<ISSingleton> imag =
        input.getImag()->asSingleton()->pipeline().transform<ISSingleton, ISSingleton, FFTPermute>
        ([=] (ISSingleton& input, ISSingleton& output) {
            output.setup<ISRe16Rgba>(width, height);
        }, FFTPermute(width, height,
                      FFTPermute::Stride::SkipNone,
                      FFTPermute::Offset::Zero,
                      orientation, plan)
         ).result<ISSingleton>();
        output.setup(real, imag);
    };
}

function<void (ISComplex&, ISComplex&)> butterflyStage(GLuint width, GLuint height, FFTSubBlock::Orientation orientation, GLuint b1, GLuint b2, GLuint blockCapacity, int sign)
{
    return [=] (ISComplex& input, ISComplex& output) {
        
        unique_ptr<ISSingleton> real =
        input.pipeline().transform<ISComplex, ISSingleton>
        ([=] (ISSingleton& output) {
            output.setup<ISRe16Rgba>(width, height);
        },
         [=] (ISSingleton& output, ISPipeline& pipeline){
             for (GLuint s = 0; s < b1; s++) {
                 pipeline.drawablePass<ISComplex, ISSingleton, FFTSubBlock>(output, FFTSubBlock(width, height, orientation, b1, b2, blockCapacity, s, FFTSubBlock::OutType::Real, sign));
             }
         }).result<ISSingleton>();
        
        unique_ptr<ISSingleton> imag =
        input.pipeline().transform<ISComplex, ISSingleton>
        ([=] (ISSingleton& output) {
            output.setup<ISRe16Rgba>(width, height);
        },
         [=] (ISSingleton& output, ISPipeline& pipeline){
             for (GLuint s = 0; s < b1; s++) {
                 pipeline.drawablePass<ISComplex, ISSingleton, FFTSubBlock>(output, FFTSubBlock(width, height, orientation, b1, b2, blockCapacity, s, FFTSubBlock::OutType::Imag, sign));
             }
         }).result<ISSingleton>();
        output.setup(real, imag);
    };
}
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
        pipeline.transform<ISComplex, ISComplex>(butterflyStage(width, height, orientation, factors[b], factors[b+1], blockCapacity, sign));
        blockCapacity*=factors[b];
    }
    pipeline.transform<ISComplex, ISComplex>(butterflyStage(width, height, orientation, factors.back(), 0, blockCapacity, sign));
}
function<void (ISComplex&, ISComplex&)> realTransform(GLuint width, GLuint height, int direction)
{
    return [=] (ISComplex& input, ISComplex& output) {
        unique_ptr<ISSingleton> real =
        input.pipeline().transform<ISComplex, ISSingleton>
        ([=] (ISSingleton& output) {
            output.setup<ISRe16Rgba>(width, height);
        }, [=] (ISSingleton& output, ISPipeline& pipeline) {
            pipeline.drawablePass<ISComplex, ISSingleton>(output, FFTRealTransformNDC(width, height, FFTRealTransformNDC::OutType::Real, direction));
            pipeline.drawablePass<ISComplex, ISSingleton>(output, FFTRealTransformHighFreq(width, height, FFTRealTransformHighFreq::OutType::Real, direction));
            pipeline.drawablePass<ISComplex, ISSingleton>(output, FFTRealTransformLowFreq(width, height, FFTRealTransformLowFreq::OutType::Real, direction));

        }).result<ISSingleton>();
        
        unique_ptr<ISSingleton> imag =
        input.pipeline().transform<ISComplex, ISSingleton>
        ([=] (ISSingleton& output) {
            output.setup<ISRe16Rgba>(width, height);
        }, [=] (ISSingleton& output, ISPipeline& pipeline) {
            pipeline.drawablePass<ISComplex, ISSingleton>(output, FFTRealTransformNDC(width, height, FFTRealTransformNDC::OutType::Imag, direction));
            pipeline.drawablePass<ISComplex, ISSingleton>(output, FFTRealTransformLowFreq(width, height, FFTRealTransformLowFreq::OutType::Imag, direction));
            pipeline.drawablePass<ISComplex, ISSingleton>(output, FFTRealTransformHighFreq(width, height, FFTRealTransformHighFreq::OutType::Imag, direction));
        }).result<ISSingleton>();
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
- (void)viewDidLoad
{
    [super viewDidLoad];

    //Make context, set current
    EAGLContext* context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:context];
    NSString* path = [[NSBundle mainBundle] pathForResource:@"cyberpunk" ofType:@"png"];
    UIImage* image = [UIImage imageWithContentsOfFile:path];
    NSError* err;
    NSDictionary *options = @{GLKTextureLoaderOriginBottomLeft:@NO}; //Image data not flipped.
    GLKTextureInfo* textureInfo;
    textureInfo = [GLKTextureLoader textureWithCGImage:image.CGImage options:options error:&err];
    NSLog(@"Loaded image into %d", textureInfo.name);
    int w = image.size.width;
    int h = image.size.height;
    vector<GLuint> factors = collectTwos(factorInteger(w/2));
    for (int num : factors) {
        cout << num << ", ";
    }
    
    ISTextureRef tex = ISURe8Rgba::fromExisting(textureInfo.name, w, h, GL_UNSIGNED_BYTE);
    ISPipeline pipeline(unique_ptr<ISSingleton>(tex->asSingleton()));
    pipeline.setupRoot();

    //Forward real 2d transform
    pipeline.transform<ISSingleton, ISComplex>(permuteEvenToRealAndOddToImag(w/2, h, FFTPermute::Orientation::Cols, factors));
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Cols, 1, factors);

    auto scale = [=] (ISComplex& input, ISComplex& output) {
        unique_ptr<ISSingleton> real =
        input.getReal()->asSingleton()->pipeline().transform<ISSingleton, ISSingleton, ISPassThroughDrawable>
        ([=](ISSingleton& input, ISSingleton& output) {
            output.setup<ISRe16Rgba>(w/2, h);
        }, ISPassThroughDrawable(w/2, h, 2.0/w)).result<ISSingleton>();
        
        unique_ptr<ISSingleton> imag =
        input.getImag()->asSingleton()->pipeline().transform<ISSingleton, ISSingleton, ISPassThroughDrawable>
        ([=](ISSingleton& input, ISSingleton& output) {
            output.setup<ISRe16Rgba>(w/2, h);
        }, ISPassThroughDrawable(w/2, h, 2.0/w)).result<ISSingleton>();
        output.setup(real, imag);
    };
    //Without intermediate scaling the values are saturating the half floats on the phone
    //This could be integrated into the non phase correcting pass, or multiply by 1/sqrt{R_{M-b} at each stage
    pipeline.transform<ISComplex, ISComplex>(scale);
    
    vector<GLuint> factorsv = collectTwos(factorInteger(h));
    pipeline.transform<ISComplex, ISComplex>(permuteComplex(w/2, h, FFTPermute::Orientation::Rows, factorsv));
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Rows, 1, factorsv);


    //Convert interleave to FT spectrum and back, don't forget to create the phase tables in the cache first
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FFTPhaseTable::renderPhaseTable(w/2, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2, w, -1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, 1);
    FFTPhaseTable::renderPhaseTable(w/2+1, w, -1);
    glBindFramebuffer(GL_FRAMEBUFFER, ISPipeline::_framebufferName);

    //TODO: verify the FFT is right, not just invertible. Use the print phase table to show a list of floats and compare to mathematica
    //If the transform is still invertible it might be a problem with the phase table.
    //TODO: This is "working" need another verification that the phase table is actually correct
    //Incorrect phase table use still allows the real transform to be invertible if it's incorrect on each side
//    pipeline.transform<ISComplex, ISComplex>(realTransform(w/2, h, 1));
    
    glClearColor(0.0, 0.0, 0.0, 1.0);
    //Gaussian filter
    pipeline.transform<ISComplex, ISComplex>
    ([=](ISComplex& input, ISComplex& output) {
        unique_ptr<ISSingleton> real =
        input.getReal()->asSingleton()->pipeline().transform<ISSingleton, ISSingleton, FFTGaussianFilter>
        ([=] (ISSingleton& input, ISSingleton& output) {
            output.setup<ISRe16Rgba>(w/2, h);
        }, FFTGaussianFilter(w/2, h, 50)).result<ISSingleton>();
        unique_ptr<ISSingleton> imag =
        input.getImag()->asSingleton()->pipeline().transform<ISSingleton, ISSingleton, FFTGaussianFilter>
        ([=] (ISSingleton& input, ISSingleton& output) {
            output.setup<ISRe16Rgba>(w/2, h);
        }, FFTGaussianFilter(w/2, h, 50)).result<ISSingleton>();
        output.setup(real, imag);
    });
    /*
    pipeline.transform<ISComplex, ISComplex>(realTransform(w/2, h, -1));

    //Inverse 2D

    pipeline.transform<ISComplex, ISComplex>(permuteComplex(w/2, h, FFTPermute::Orientation::Cols, factors));
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Cols, -1, factors);
    pipeline.transform<ISComplex, ISComplex>(permuteComplex(w/2, h, FFTPermute::Orientation::Rows, factorsv));
    butterflyAll(pipeline, w/2, h, FFTSubBlock::Orientation::Rows, -1, factorsv);
*/
    
    //Read out result
    auto selectRealDiscardComplex = [=] (ISComplex& input, ISSingleton& output) {
        unique_ptr<ISSingleton> real =
        input.getReal()->asSingleton()->pipeline().transform<ISSingleton, ISSingleton, ISPassThroughDrawable>
        ([=] (ISSingleton& input, ISSingleton& output) {
            output.setup<ISURe8Rgba>(w, h);
        }, ISPassThroughDrawable(w, h, 1.0f)).result<ISSingleton>();
        output.setup(real);
    };
    pipeline.transform<ISComplex, ISSingleton>(selectRealDiscardComplex);
    
    GLint readType;
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &readType);
    NSLog(@"Supported read type %d (expecting %d)", readType, GL_UNSIGNED_BYTE);
    GLubyte *tempImagebuffer = (GLubyte *) malloc(w*h*4);
    glReadPixels( 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, tempImagebuffer);
    
    // make data provider with data.
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, tempImagebuffer, w*h*4, ProviderReleaseData);
    
    // prep the ingredients
    int bitsPerComponent = 8;
    int bitsPerPixel = 32;
    int bytesPerRow = 4*w;
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    // make the cgimage
    CGImageRef imageRef = CGImageCreate(w, h, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);
    
    // then make the uiimage from that
    UIImage *myImage =  [UIImage imageWithCGImage:imageRef] ;
    _imageView.image = myImage;
    CGDataProviderRelease(provider);
    CGImageRelease(imageRef);
    CGColorSpaceRelease(colorSpaceRef);
    [UIImagePNGRepresentation(myImage) writeToFile:@"/Users/williamtalmadge/Downloads/ISFFT.png" atomically:YES];
    
    pipeline.releaseAllCaches();
    pipeline.teardown();
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
