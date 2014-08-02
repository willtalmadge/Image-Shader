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

#ifndef __Image_Shader__PDESetBoundary__
#define __Image_Shader__PDESetBoundary__

#include <functional>
#include <vector>
#include "ISSingleton.h"
#include "arm_neon.h"

std::function<void (ISSingleton&, ISRect, ISRect)>
extractBoundary_Re16RgbaVFP4(std::vector<float16x4_t>& boundary);
std::function<void (ISSingleton&, ISRect, ISRect)>
setBoundary_Re16RgbaVFP4(const std::vector<float16x4_t>& boundary);
std::function<void (ISSingleton&, ISRect, ISRect)>
setBoundaryTopCubicInterpXY_Re16RgbaVFP4(const std::vector<GLfloat>& boundaryX, const std::vector<GLfloat>& boundaryY);
std::function<void (ISSingleton&, ISRect, ISRect)>
setBoundaryRightCubicInterpXY_Re16RgbaVFP4(const std::vector<GLfloat>& boundaryX, const std::vector<GLfloat>& boundaryY);
std::function<void (ISSingleton&, ISRect, ISRect)>
setBoundaryBottomCubicInterpXY_Re16RgbaVFP4(const std::vector<GLfloat>& boundaryX, const std::vector<GLfloat>& boundaryY);
std::function<void (ISSingleton&, ISRect, ISRect)>
setBoundaryLeftCubicInterpXY_Re16RgbaVFP4(const std::vector<GLfloat>& boundaryX, const std::vector<GLfloat>& boundaryY);
std::function<void (ISSingleton&, ISRect, ISRect)>makeHomogeneousXY_Re16RgbaVFP4();
#endif /* defined(__Image_Shader__PDESetBoundary__) */
