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
#import <fstream>
#import "ISPassThroughDrawable.h"
#import "ISPipelineBufferable.h"
#import "ISComplex.h"
#import "FFTTransformers.h"
#import "FSTPrecondition.h"
#import "FSTFinalize.h"
#import "PDESetBoundary.h"
#import "ISURe8Rgba.h"
#import "ISRe16Rgba.h"
#import "ISRemappable.h"
#import "ISRemap.h"
#import "half.hpp"
#import <arm_neon.h>

#import "opencv2/opencv.hpp"

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


void readFBRow(GLuint width, GLuint y0)
{
    GLfloat *r = new GLfloat[4*width];
    glReadPixels(0, y0, width, 1, GL_RGBA, GL_HALF_FLOAT_OES, r);
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
template <class T>
void read(ifstream& i, T* t) {
    i.read((char*)t, sizeof(T));
}
- (IBAction)remap:(id)sender
{
    //Load a boundary curve with normalized coordinates
    NSString* path = [[NSBundle mainBundle] pathForResource:@"curveCubicExtrapNormalizedCoords" ofType:@"bin"];
    ifstream curveF(path.UTF8String, ios::in | ios::binary);
    
    vector<float> topX, topY, rightX, rightY, bottomX, bottomY, leftX, leftY;
    uint32_t n;
    read(curveF, &n);
    for (int i = 0; i < n; i++) {
        array<float, 2> v;
        read(curveF, &v);
        topX.push_back(v[0]);
        topY.push_back(v[1]);
    }
    read(curveF, &n);
    for (int i = 0; i < n; i++) {
        array<float, 2> v;
        read(curveF, &v);
        rightX.push_back(v[0]);
        rightY.push_back(v[1]);
    }
    read(curveF, &n);
    for (int i = 0; i < n; i++) {
        array<float, 2> v;
        read(curveF, &v);
        bottomX.push_back(v[0]);
        bottomY.push_back(v[1]);
    }
    read(curveF, &n);
    for (int i = 0; i < n; i++) {
        array<float, 2> v;
        read(curveF, &v);
        leftX.push_back(v[0]);
        leftY.push_back(v[1]);
    }
    //Setup a half float pipeline
    EAGLContext* context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:context];

    
    UIImage* image = _imageView.image;
    NSError* err;
    NSDictionary *options = @{GLKTextureLoaderOriginBottomLeft:@NO}; //Image data not flipped.
    GLKTextureInfo* textureInfo;
    textureInfo = [GLKTextureLoader textureWithCGImage:image.CGImage options:options error:&err];
    
    ISPipelineBufferable::setContext(context);
    int w = image.size.width;
    int h = image.size.height;
    
    ISTextureRef tex = ISURe8Rgba<ISPBuffer>::fromExisting(textureInfo.name, w, h, GL_UNSIGNED_BYTE);
    ISPipelineBufferable pipeline(unique_ptr<ISSingleton>(tex->asSingleton()));
    pipeline.setContext(context);
    pipeline.setupRoot();
    
    pipeline.transform<ISSingleton, ISRemappable>
    ([topX, topY, rightX, rightY, bottomX, bottomY, leftX, leftY]
     (ISSingleton& input, ISRemappable& output, ISRect sourceROI, ISRect targetROI) {
        int w = 1001;
        int h = 1001;
        ISTextureRef tex = ISRe16Rgba<ISPBuffer>::make(w, h);
        ISPipelineBufferable pipeline(unique_ptr<ISSingleton>(tex->asSingleton())); //I haven't tried opening a pipeline on a raw texture like this yet. I could instead build this outside this transformer and form an ISRemappable outside and initialize the root pipeline with a remappable.
        pipeline.transformInPlace
        (setBoundaryTopCubicInterpXY_Re16RgbaVFP4(topX, topY));
        pipeline.transformInPlace
        (setBoundaryRightCubicInterpXY_Re16RgbaVFP4(rightX, rightY));
        pipeline.transformInPlace
        (setBoundaryBottomCubicInterpXY_Re16RgbaVFP4(bottomX, bottomY));
        pipeline.transformInPlace
        (setBoundaryLeftCubicInterpXY_Re16RgbaVFP4(leftX, leftY));
        
        vector<float16x4_t> boundary;
        pipeline.transformInPlace(extractBoundary_Re16RgbaVFP4(boundary));
        pipeline.transformInPlace(makeHomogeneousXY_Re16RgbaVFP4());
        //Do Poisson filter here
        poisson2D(pipeline, ISRect(0, w-1, 0, h-1)); //Refactor this into poisson2D and make this back into a simple fst test
        pipeline.fullROI()
                .transformInPlace(setBoundary_Re16RgbaVFP4(boundary));
        std::cout << input._elements.size() << endl;

        unique_ptr<ISSingleton> remap = pipeline.result<ISSingleton>(); //How is the memory management of texture refs and tuples for this construct?
        output.setup(input.getTexture(), remap->getTexture());
         //The input is passed for two purposes, to transform or to pass through into a new tuple, so setup should tie textures together, not singletons as it does now.
         //FIXME: eliminate setup that takes singletons. Requiring singletons encourages creating a pipeline from the input and then releasing the result and storing it in a unique_ptr, when the unique_ptr is deleted it has a handle on the enclosing pipeline value and deletes the tuple prematurely.
    });
    //FIXME: can't handle URe8
    pipeline.transform<ISRemappable, ISRe16Rgba, ISRemap, ISPBuffer>(ISRemap());

    pipeline.transform<ISSingleton, ISURe8Rgba, ISPassThroughDrawable, ISTexture>
    (ISPassThroughDrawable(1.0f));
    
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
    
    //FIXME: cache not created with only pbuffers at start
    //pipeline.releaseAllCaches();
    //pipeline.teardown();
}
- (IBAction)gaussianFilter:(id)sender
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
    
    ISTextureRef tex = ISURe8Rgba<>::fromExisting(textureInfo.name, w, h, GL_UNSIGNED_BYTE);
    ISPipelineBufferable pipeline(unique_ptr<ISSingleton>(tex->asSingleton()));
    pipeline.setContext(context);
    pipeline.setupRoot();
    
    //Read out real channel result
    
    auto selectRealDiscardComplex = [=] (ISComplex& input, ISSingleton& output, ISRect sourceROI, ISRect targetROI) {
        unique_ptr<ISSingleton> real =
        input.getReal()->asSingleton()->pipeline().sourceToTargetSizeMult(2, 1).transform<ISSingleton, ISURe8Rgba, ISPassThroughDrawable, ISTexture>
        (ISPassThroughDrawable(1.0f)).result<ISSingleton>();
        output.setup(real);
    };
    
    gaussianBlur<ISTexture>(pipeline, 5);

    pipeline.transform<ISComplex, ISSingleton>(selectRealDiscardComplex);
    
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
    pipeline.teardown();
}
- (IBAction)fst:(id)sender
{
    EAGLContext* context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:context];
    
    UIImage* image = _imageView.image;
    NSError* err;
    NSDictionary *options = @{GLKTextureLoaderOriginBottomLeft:@NO}; //Image data not flipped.
    GLKTextureInfo* textureInfo;
    textureInfo = [GLKTextureLoader textureWithCGImage:image.CGImage options:options error:&err];
    int w = image.size.width;
    int h = image.size.height;
    
    ISPipelineBufferable::setContext(context);
    ISTextureRef tex = ISURe8Rgba<>::fromExisting(textureInfo.name, w, h, GL_UNSIGNED_BYTE);
    ISPipelineBufferable pipeline(unique_ptr<ISSingleton>(tex->asSingleton()));
    pipeline.setContext(context);
    pipeline.setupRoot();
    
    //w = 2048;
    //h = 2048;
    pipeline.setTargetSize(w, h)
            .fullROI()
            .transform<ISSingleton, ISRe16Rgba, ISPassThroughDrawable, ISPBuffer>
            (ISPassThroughDrawable());

    fstInversionTestRowMajor1D(pipeline);
    pipeline.transform(selectRealDiscardComplexDoubleRows());

    pipeline.setTargetSize(w, h)
            .fullROI()
            .transform<ISSingleton, ISURe8Rgba, ISPassThroughDrawable, ISTexture>
    (ISPassThroughDrawable());
    
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
    int w = image.size.width;
    int h = image.size.height;
    
    ISPipelineBufferable::setContext(context);
    ISTextureRef tex = ISURe8Rgba<>::fromExisting(textureInfo.name, w, h, GL_UNSIGNED_BYTE);
    ISPipelineBufferable pipeline(unique_ptr<ISSingleton>(tex->asSingleton()));
    pipeline.setContext(context);
    pipeline.setupRoot();
    w = 100;
    h = 100;
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

    

    pipeline.setTargetSize(w, h)
    .fullROI()
    .transform<ISSingleton, ISRe16Rgba, ISPassThroughDrawable, ISPBuffer>
    (ISPassThroughDrawable());
    
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    pipeline.transform<ISSingleton, ISRe16Rgba, FSTPrecondition, ISPBuffer>
    (FSTPrecondition(FSTPrecondition::Orientation::Rows));
    
    pipeline.transform<ISSingleton, ISURe8Rgba, ISPassThroughDrawable, ISTexture>
    (ISPassThroughDrawable(1.0f));
    
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
    
    //FIXME: cache not created with only pbuffers at start
    //pipeline.releaseAllCaches();
    //pipeline.teardown();
}
- (void)viewDidLoad
{
    [super viewDidLoad];

    NSString* path = [[NSBundle mainBundle] pathForResource:@"page" ofType:@"jpg"];
    UIImage* image = [UIImage imageWithContentsOfFile:path];
    _imageView.image = image;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
