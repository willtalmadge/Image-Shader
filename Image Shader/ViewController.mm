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
#import "ISPipelineBufferable.h"
#import "ISComplex.h"
#import "FFTTransformers.h"
#import "FSTPrecondition.h"
#import "FSTFinalize.h"
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

    
    vector<GLuint> factors = collectTwos(factorInteger(w/2));
    for (int num : factors) {
        cout << num << ", ";
    }
    
    ISTextureRef tex = ISURe8Rgba<>::fromExisting(textureInfo.name, w, h, GL_UNSIGNED_BYTE);
    ISPipelineBufferable pipeline(unique_ptr<ISSingleton>(tex->asSingleton()));
    pipeline.setContext(context);
    pipeline.setupRoot();

    //Read out real channel result

    auto selectRealDiscardComplex = [=] (ISComplex& input, ISSingleton& output) {
        unique_ptr<ISSingleton> real =
        input.getImag()->asSingleton()->pipeline().transform<ISSingleton, ISURe8Rgba, ISPassThroughDrawable, ISTexture>
        (w, h,
         ISPassThroughDrawable(w, h, 1.0f)).result<ISSingleton>();
        output.setup(real);
    };

//    gaussianBlur<ISTexture>(pipeline, w, h, 5);
//    rftInversionTestRowMajor1D<ISTexture>(pipeline, w, h);
    fstInversionTest2D(pipeline, w, h);
//    readFBRow(w, h);
//    pipeline.transform<ISComplex, ISSingleton>(selectRealDiscardComplex);
    pipeline.transform<ISSingleton, ISURe8Rgba, ISPassThroughDrawable, ISTexture>
    (w, h,
        ISPassThroughDrawable(w, h, 1.0f));
    
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
