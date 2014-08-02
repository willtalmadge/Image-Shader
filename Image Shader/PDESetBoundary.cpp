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

#include "PDESetBoundary.h"
#include "ISPBuffer.h"

using namespace std;

std::function<void (ISSingleton&, ISRect, ISRect)>
extractBoundary_Re16RgbaVFP4(std::vector<float16x4_t>& boundary)
{
    return [=, &boundary] (ISSingleton& input, ISRect sourceROI, ISRect targetROI) {
        assert(sourceROI.left() == 0); //TODO: make this work on arbitrary source roi
        assert(sourceROI.top() == 0);
        uint width = sourceROI.width();
        uint height = sourceROI.height();
        boundary.resize(2*width + 2*height - 4);
        ISPBufferRef in = dynamic_cast<ISPBufferRef>(input.getTexture());
        float16x4_t* inp = (float16x4_t*)in->baseAddress();
        
        //Input indexer
        size_t Ni = in->bytesPerRow()/sizeof(float16x4_t);
        auto iidx = [=] (size_t r, size_t c) -> size_t {
            return Ni*r + c;
        };
        
        vector<float16x4_t>::iterator point = boundary.begin();
        for (int i = 0; i < width; i++) {
            //Across the top to the right
            *point = inp[iidx(0, i)];
            ++point;
        }
        
        for (int i = 1; i < height; i++) {
            //Down the right side
            *point = inp[iidx(i, width - 1)];
            ++point;
        }
        
        for (int i = width - 2; i >= 0; i--) {
            //Across the bottom to the left
            *point = inp[iidx(height - 1, i)];
            ++point;
        }
        
        for (int i = height - 2; i > 0; i--) {
            //Up the left
            *point = inp[iidx(i, 0)];
            ++point;
        }
    };
}
std::function<void (ISSingleton&, ISRect, ISRect)>
setBoundary_Re16RgbaVFP4(const std::vector<float16x4_t>& boundary)
{
    return [=, &boundary] (ISSingleton& input, ISRect sourceROI, ISRect targetROI) {
        assert(sourceROI.left() == 0); //TODO: make this work on arbitrary source roi
        assert(sourceROI.top() == 0);
        uint width = sourceROI.width();
        uint height = sourceROI.height();
        ISPBufferRef in = dynamic_cast<ISPBufferRef>(input.getTexture());
        float16x4_t* inp = (float16x4_t*)in->baseAddress();
        
        //Input indexer
        size_t Ni = in->bytesPerRow()/sizeof(float16x4_t);
        auto iidx = [=] (size_t r, size_t c) -> size_t {
            return Ni*r + c;
        };
        
        vector<float16x4_t>::const_iterator point = boundary.begin();
        for (int i = 0; i < width; i++)
        {
            //Across the top to the right
            inp[iidx(0, i)] = *point;
            point++;
        }
        for (int i = 1; i < height; i++)
        {
            //Down the right side
            inp[iidx(i, width - 1)] = *point;
            point++;
        }
        for (int i = width - 2; i >= 0; i--)
        {
            //Across the bottom to the left
            inp[iidx(height - 1, i)] = *point;
            point++;
        }
        
        for (int i = height - 2; i > 0; i--)
        {
            //Up the left
            inp[iidx(i, 0)] = *point;
            point++;
        }
        assert(point == boundary.end());
    };
}
float cubicInterp(float y0, float y1, float y2, float y3, float mu)
{
    float mu2 = mu*mu;
    float a0 = y3 - y2 - y0 + y1;
    float a1 = y0 - y1 - a0;
    float a2 = y2 - y0;
    float a3 = y1;
    
    return a0*mu*mu2 + a1*mu2 + a2*mu + a3;
}
float cubicInterp(const vector<float>& extrapolatedData, float normalizedCoord)
{
    //This does not handle the endpoint extrapolation. The input must be extrapolated
    //if you want to interpolate up to the start and end of the source vector.
    assert(extrapolatedData.size() > 3);
    assert(normalizedCoord >= 0.0f && normalizedCoord <= 1.0f);
    float t = normalizedCoord*(extrapolatedData.size() - 3) + 1;
    float n = floor(t);
    float mu = t - n;
    
    float y0 = extrapolatedData[n - 1];
    float y1 = extrapolatedData[n];
    float y2 = extrapolatedData[n + 1];
    float y3 = extrapolatedData[n + 2];
    
    return cubicInterp(y0, y1, y2, y3, mu);
}

std::function<void (ISSingleton&, ISRect, ISRect)>
setBoundaryTopCubicInterpXY_Re16RgbaVFP4(const vector<GLfloat>& boundaryX, const vector<GLfloat>& boundaryY)
{
    //boundaryX and boundaryY must have an extra extrapolated data point on the start and end of the sequence.
    return [=] (ISSingleton& input, ISRect sourceROI, ISRect targetROI) {
        assert(sourceROI.left() == 0); //TODO: make this work on arbitrary source roi
        assert(sourceROI.top() == 0);
        uint width = sourceROI.width();
        ISPBufferRef in = dynamic_cast<ISPBufferRef>(input.getTexture());
        float16x4_t* inp = (float16x4_t*)in->baseAddress();
        
        //Input indexer
        size_t Ni = in->bytesPerRow()/sizeof(float16x4_t);
        auto iidx = [=] (size_t r, size_t c) -> size_t {
            return Ni*r + c;
        };
        
        for (int i = 0; i < width; i++)
        {
            //Across the top to the right
            float32x4_t pixel = {0.0f, 0.0f, 0.0f, 0.0f};
            float32_t* pixelp = (float32_t*)&pixel;
            float u = static_cast<float>(i)/width;
            pixelp[0] = cubicInterp(boundaryX, u);
            pixelp[1] = cubicInterp(boundaryY, u);
            inp[iidx(0, i)] = vcvt_f16_f32(pixel);
        }
    };
}
std::function<void (ISSingleton&, ISRect, ISRect)>
setBoundaryRightCubicInterpXY_Re16RgbaVFP4(const vector<GLfloat>& boundaryX, const vector<GLfloat>& boundaryY)
{
    //boundaryX and boundaryY must have an extra extrapolated data point on the start and end of the sequence.
    return [=] (ISSingleton& input, ISRect sourceROI, ISRect targetROI) {
        assert(sourceROI.left() == 0); //TODO: make this work on arbitrary source roi
        assert(sourceROI.top() == 0);
        uint width = sourceROI.width();
        uint height = sourceROI.height();
        ISPBufferRef in = dynamic_cast<ISPBufferRef>(input.getTexture());
        float16x4_t* inp = (float16x4_t*)in->baseAddress();
        
        //Input indexer
        size_t Ni = in->bytesPerRow()/sizeof(float16x4_t);
        auto iidx = [=] (size_t r, size_t c) -> size_t {
            return Ni*r + c;
        };
        for (int i = 0; i < height; i++)
        {
            //Across the top to the right
            float32x4_t pixel = {0.0f, 0.0f, 0.0f, 0.0f};
            float32_t* pixelp = (float32_t*)&pixel;
            float u = static_cast<float>(i)/height;
            pixelp[0] = cubicInterp(boundaryX, u);
            pixelp[1] = cubicInterp(boundaryY, u);
            inp[iidx(i, width - 1)] = vcvt_f16_f32(pixel);
        }
    };
}
std::function<void (ISSingleton&, ISRect, ISRect)>
setBoundaryBottomCubicInterpXY_Re16RgbaVFP4(const vector<GLfloat>& boundaryX, const vector<GLfloat>& boundaryY)
{
    //boundaryX and boundaryY must have an extra extrapolated data point on the start and end of the sequence.
    //Runs right to left
    return [=] (ISSingleton& input, ISRect sourceROI, ISRect targetROI) {
        assert(sourceROI.left() == 0); //TODO: make this work on arbitrary source roi
        assert(sourceROI.top() == 0);
        uint width = sourceROI.width();
        uint height = sourceROI.height();
        ISPBufferRef in = dynamic_cast<ISPBufferRef>(input.getTexture());
        float16x4_t* inp = (float16x4_t*)in->baseAddress();
        
        //Input indexer
        size_t Ni = in->bytesPerRow()/sizeof(float16x4_t);
        auto iidx = [=] (size_t r, size_t c) -> size_t {
            return Ni*r + c;
        };
        
        for (int i = 0; i < width; i++)
        {
            //Across the top to the right
            float32x4_t pixel = {0.0f, 0.0f, 0.0f, 0.0f};
            float32_t* pixelp = (float32_t*)&pixel;
            float u = static_cast<float>(i)/width;
            pixelp[0] = cubicInterp(boundaryX, u);
            pixelp[1] = cubicInterp(boundaryY, u);
            inp[iidx(height - 1, width - i - 1)] = vcvt_f16_f32(pixel);
        }
    };
}
std::function<void (ISSingleton&, ISRect, ISRect)>
setBoundaryLeftCubicInterpXY_Re16RgbaVFP4(const vector<GLfloat>& boundaryX, const vector<GLfloat>& boundaryY)
{
    //boundaryX and boundaryY must have an extra extrapolated data point on the start and end of the sequence.
    //Runs bottom to top
    return [=] (ISSingleton& input, ISRect sourceROI, ISRect targetROI) {
        assert(sourceROI.left() == 0); //TODO: make this work on arbitrary source roi
        assert(sourceROI.top() == 0);
        uint height = sourceROI.height();
        ISPBufferRef in = dynamic_cast<ISPBufferRef>(input.getTexture());
        float16x4_t* inp = (float16x4_t*)in->baseAddress();
        
        //Input indexer
        size_t Ni = in->bytesPerRow()/sizeof(float16x4_t);
        auto iidx = [=] (size_t r, size_t c) -> size_t {
            return Ni*r + c;
        };
        
        for (int i = 0; i < height; i++)
        {
            //Across the top to the right
            float32x4_t pixel = {0.0f, 0.0f, 0.0f, 0.0f};
            float32_t* pixelp = (float32_t*)&pixel;
            float u = static_cast<float>(i)/height;
            pixelp[0] = cubicInterp(boundaryX, u);
            pixelp[1] = cubicInterp(boundaryY, u);
            inp[iidx(height - i - 1, 0)] = vcvt_f16_f32(pixel);
        }
    };
}
void vdec1(float16x4_t* p, size_t a, size_t b)
{
    float32x4_t la = vcvt_f32_f16(p[a]);
    float32x4_t lb = vcvt_f32_f16(p[b]);
    float32x4_t v = vsubq_f32(la, lb);
    p[a] = vcvt_f16_f32(v);
}
void vdec2(float16x4_t* p, size_t a, size_t b0, size_t b1)
{
    float32x4_t la = vcvt_f32_f16(p[a]);
    float32x4_t lb0 = vcvt_f32_f16(p[b0]);
    float32x4_t lb1 = vcvt_f32_f16(p[b1]);
    float32x4_t v = vsubq_f32(la, vaddq_f32(lb0, lb1));
    p[a] = vcvt_f16_f32(v);
}
void vsetzero(float16x4_t* p, size_t a)
{
    float32x4_t v = {0.0f, 0.0f, 0.0f, 0.0f};
    p[a] = vcvt_f16_f32(v);
}
std::function<void (ISSingleton&, ISRect, ISRect)>
makeHomogeneousXY_Re16RgbaVFP4()
{
    return [=] (ISSingleton& input, ISRect sourceROI, ISRect targetROI) {
        assert(sourceROI.left() == 0); //TODO: make this work on arbitrary source roi
        assert(sourceROI.top() == 0);
        uint width = sourceROI.width();
        uint height = sourceROI.height();
        ISPBufferRef in = dynamic_cast<ISPBufferRef>(input.getTexture());
        float16x4_t* inp = (float16x4_t*)in->baseAddress();
        
        //Input indexer
        size_t Ni = in->bytesPerRow()/sizeof(float16x4_t);
        auto iidx = [=] (size_t r, size_t c) -> size_t {
            return Ni*r + c;
        };
        
        for (int i = 2; i < width - 2; i++)
        {
            vdec1(inp, iidx(1, i), iidx(0, i));

            vdec1(inp, iidx(height - 2, i), iidx(height - 1, i));
            vsetzero(inp, iidx(0, i));
            vsetzero(inp, iidx(height - 1, i));
        }
        for (int i = 2; i < height - 2; i++)
        {
            vdec1(inp, iidx(i, 1), iidx(i, 0));
            vdec1(inp, iidx(i, width - 2), iidx(i, width - 1));
            vsetzero(inp, iidx(i, 0));
            vsetzero(inp, iidx(i, width - 1));
        }
        
        //Now handle the corners
        vdec2(inp, iidx(1, 1), iidx(0, 1), iidx(1, 0));
        vdec2(inp, iidx(1, width - 2), iidx(0, width - 2), iidx(1, width - 1));
        vdec2(inp, iidx(height - 2, 1), iidx(height - 2, 0), iidx(height - 1, 1));
        vdec2(inp, iidx(height - 2, width - 2), iidx(height - 2, width - 1), iidx(height - 1, width - 2));
        
        //Set all values touched by the corners to 0
        vsetzero(inp, iidx(0, 1));
        vsetzero(inp, iidx(1, 0));
        vsetzero(inp, iidx(0, width - 2));
        vsetzero(inp, iidx(1, width - 1));
        vsetzero(inp, iidx(height - 2, 0));
        vsetzero(inp, iidx(height - 1, 1));
        vsetzero(inp, iidx(height - 2, width - 1));
        vsetzero(inp, iidx(height - 1, width - 2));
        
        vsetzero(inp, iidx(0, 0));
        vsetzero(inp, iidx(0, width - 1));
        vsetzero(inp, iidx(height - 1, 0));
        vsetzero(inp, iidx(height - 1, width - 1));
    };
}