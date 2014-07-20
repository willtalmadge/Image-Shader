//
//  FSTFinalize.h
//  Image Shader
//
//  Created by William Talmadge on 7/13/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

#ifndef __Image_Shader__FSTFinalize__
#define __Image_Shader__FSTFinalize__

#include <functional>
#include "ISSingleton.h"
#include "ISComplex.h"

enum class FSTFinalizeOrientation {Rows, Cols};
std::function<void (ISComplex&, ISSingleton&)> fstFinalizeForward_Re16RgbaVFP4(GLuint width, GLuint height, FSTFinalizeOrientation orientation);
std::function<void (ISSingleton&, ISComplex&)> fstFinalizeInverse_Re16RgbaNEON(GLuint width, GLuint height, FSTFinalizeOrientation orientation);

#endif /* defined(__Image_Shader__FSTFinalize__) */
