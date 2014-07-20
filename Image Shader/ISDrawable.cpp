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

#include "ISDrawable.h"

using namespace std;

std::unordered_set<ISDrawableRef, ISDrawableCache::drawableHash, ISDrawableCache::drawableCompare>
ISDrawableCache::_drawableCache = std::unordered_set<ISDrawableRef, ISDrawableCache::drawableHash, ISDrawableCache::drawableCompare>();

//The IS coordinate system is used for these functions. (0,0) in texture coordinates corresponds to the top left corner of the image to be processed. GL is actually processing the image up side down. But, this fact does not manifest in the result because glTexImage2D and glReadPixels flip of the y-axis cancel each other out. As a consequence we can treat a logical coordinate system in the pipeline the same as the physical coordinate. We won't notice it unless you try touching the memory directly outside of glReadPixels.
//They all use the same quad
// v0-v1
// | / |
// v3-v2
vector<GLfloat> makeGlAttributePixelColumn(GLuint length, GLuint x1, GLuint x2,
                                           const vector<GLfloat>& attributesTop,
                                           const vector<GLfloat>& attributesBottom)
{
    assert(attributesTop.size() == attributesBottom.size());
    //Layout:
    //x, y, z, t, {attributes}
    float v0[] = {static_cast<float>(x1), 0.0, 0.5};//0
    float v1[] = {static_cast<float>(x2), 0.0, 0.5};//1
    float v2[] = {static_cast<float>(x2), static_cast<float>(length), 0.5};//2
    float v3[] = {static_cast<float>(x1), static_cast<float>(length), 0.5};//3
    
    vector<GLfloat> result;
    //Top triangle
    result.reserve(6*4*attributesTop.size()*sizeof(GLfloat));
    result.insert(result.end(), v0, v0+3);
    result.insert(result.end(), attributesTop.begin(), attributesTop.end());
    result.insert(result.end(), v1, v1+3);
    result.insert(result.end(), attributesTop.begin(), attributesTop.end());
    result.insert(result.end(), v3, v3+3);
    result.insert(result.end(), attributesBottom.begin(), attributesBottom.end());
    //Bottom triangle
    result.insert(result.end(), v1, v1+3);
    result.insert(result.end(), attributesTop.begin(), attributesTop.end());
    result.insert(result.end(), v2, v2+3);
    result.insert(result.end(), attributesBottom.begin(), attributesBottom.end());
    result.insert(result.end(), v3, v3+3);
    result.insert(result.end(), attributesBottom.begin(), attributesBottom.end());
    return result;
}
void makeGlLookupColumnVarying(vector<GLfloat>& vec, GLuint length, GLuint x1, GLuint x2,
                               const vector<GLfloat>& u1s,
                               const vector<GLfloat>& u2s)
{
    //In the lookup geometry it is assumed all the attributes are u coords, they are paired with the appropriate v coords
    assert(u1s.size() == u2s.size());
    //assert(x2 > x1);
    //Layout:
    //x, y, z, {attributes}
    float v0[] = {static_cast<float>(x1), 0.0, 0.5};//0
    float v1[] = {static_cast<float>(x2), 0.0, 0.5};//1
    float v2[] = {static_cast<float>(x2), static_cast<float>(length), 0.5};//2
    float v3[] = {static_cast<float>(x1), static_cast<float>(length), 0.5};//3
    
    //Top triangle
    vec.insert(vec.end(), v0, v0+3);
    for ( GLfloat attr : u1s) { vec.insert(vec.end(), {attr, 0.0}); }
    vec.insert(vec.end(), v1, v1+3);
    for ( GLfloat attr : u2s) { vec.insert(vec.end(), {attr, 0.0}); }
    vec.insert(vec.end(), v3, v3+3);
    for ( GLfloat attr : u1s) { vec.insert(vec.end(), {attr, 1.0}); }
    //Bottom triangle
    vec.insert(vec.end(), v1, v1+3);
    for ( GLfloat attr : u2s) { vec.insert(vec.end(), {attr, 0.0}); }
    vec.insert(vec.end(), v2, v2+3);
    for ( GLfloat attr : u2s) { vec.insert(vec.end(), {attr, 1.0}); }
    vec.insert(vec.end(), v3, v3+3);
    for ( GLfloat attr : u1s) { vec.insert(vec.end(), {attr, 1.0}); }
}
void appendGlLookupCol(vector<GLfloat>& vec, GLuint length, GLuint x1, GLuint x2,
                       const vector<GLfloat>& u1s,
                       const vector<GLfloat>& u2s,
                       const vector<GLfloat>& attr1,
                       const vector<GLfloat>& attr2)
{
    //In the lookup geometry it is assumed all the attributes are u coords, they are paired with the appropriate v coords
    assert(u1s.size() == u2s.size());
    //assert(x2 > x1);
    //Layout:
    //x, y, z, {attributes}
    float v0[] = {static_cast<float>(x1), 0.0, 0.5};//0
    float v1[] = {static_cast<float>(x2), 0.0, 0.5};//1
    float v2[] = {static_cast<float>(x2), static_cast<float>(length), 0.5};//2
    float v3[] = {static_cast<float>(x1), static_cast<float>(length), 0.5};//3
    
    //Top triangle
    vec.insert(vec.end(), v0, v0+3);
    for ( GLfloat attr : u1s) { vec.insert(vec.end(), {attr, 0.0}); }
    vec.insert(vec.end(), attr1.begin(), attr1.end());
    
    vec.insert(vec.end(), v1, v1+3);
    for ( GLfloat attr : u2s) { vec.insert(vec.end(), {attr, 0.0}); }
    vec.insert(vec.end(), attr2.begin(), attr2.end());
    
    vec.insert(vec.end(), v3, v3+3);
    for ( GLfloat attr : u1s) { vec.insert(vec.end(), {attr, 1.0}); }
    vec.insert(vec.end(), attr1.begin(), attr1.end());
    
    //Bottom triangle
    vec.insert(vec.end(), v1, v1+3);
    for ( GLfloat attr : u2s) { vec.insert(vec.end(), {attr, 0.0}); }
    vec.insert(vec.end(), attr2.begin(), attr2.end());
    
    vec.insert(vec.end(), v2, v2+3);
    for ( GLfloat attr : u2s) { vec.insert(vec.end(), {attr, 1.0}); }
    vec.insert(vec.end(), attr2.begin(), attr2.end());
    
    vec.insert(vec.end(), v3, v3+3);
    for ( GLfloat attr : u1s) { vec.insert(vec.end(), {attr, 1.0}); }
    vec.insert(vec.end(), attr1.begin(), attr1.end());
}
void appendGlLookupRow(vector<GLfloat>& vec, GLuint length, GLuint y1, GLuint y2,
                       const vector<GLfloat>& v1s,
                       const vector<GLfloat>& v2s,
                       const vector<GLfloat> attr1,
                       const vector<GLfloat> attr2)
{
    assert(v1s.size() == v2s.size());
    //assert(y2 > y1);
    //Layout:
    //x, y, z, {attributes}
    float v0[] = {0.0, static_cast<float>(y1), 0.5};//0
    float v1[] = {static_cast<float>(length), static_cast<float>(y1), 0.5};//1
    float v2[] = {static_cast<float>(length), static_cast<float>(y2), 0.5};//2
    float v3[] = {0.0, static_cast<float>(y2), 0.5};//3
    
    //Top triangle
    vec.insert(vec.end(), v0, v0+3);
    for ( GLfloat attr : v1s) { vec.insert(vec.end(), {0.0, attr}); }
    vec.insert(vec.end(), attr1.begin(), attr1.end());
    
    vec.insert(vec.end(), v1, v1+3);
    for ( GLfloat attr : v1s) { vec.insert(vec.end(), {1.0, attr}); }
    vec.insert(vec.end(), attr1.begin(), attr1.end());
    
    vec.insert(vec.end(), v3, v3+3);
    for ( GLfloat attr : v2s) { vec.insert(vec.end(), {0.0, attr}); }
    vec.insert(vec.end(), attr2.begin(), attr2.end());
    //Bottom triangle
    vec.insert(vec.end(), v1, v1+3);
    for ( GLfloat attr : v1s) { vec.insert(vec.end(), {1.0, attr}); }
    vec.insert(vec.end(), attr1.begin(), attr1.end());
    
    vec.insert(vec.end(), v2, v2+3);
    for ( GLfloat attr : v2s) { vec.insert(vec.end(), {1.0, attr}); }
    vec.insert(vec.end(), attr2.begin(), attr2.end());
    
    vec.insert(vec.end(), v3, v3+3);
    for ( GLfloat attr : v2s) { vec.insert(vec.end(), {0.0, attr}); }
    vec.insert(vec.end(), attr2.begin(), attr2.end());
}
void makeGlAttributeColumnVarying(vector<GLfloat>& vec, GLuint length, GLuint x1, GLuint x2,
                                  const vector<GLfloat>& attributes1,
                                  const vector<GLfloat>& attributes2)
{
    assert(attributes1.size() == attributes2.size());
    assert(x2 > x1);
    //Layout:
    //x, y, z, {attributes}
    float v0[] = {static_cast<float>(x1), 0.0, 0.5};//0
    float v1[] = {static_cast<float>(x2), 0.0, 0.5};//1
    float v2[] = {static_cast<float>(x2), static_cast<float>(length), 0.5};//2
    float v3[] = {static_cast<float>(x1), static_cast<float>(length), 0.5};//3
    
    //Top triangle
    vec.insert(vec.end(), v0, v0+3);
    for ( GLfloat attr : attributes1) { vec.insert(vec.end(), attr); }
    vec.insert(vec.end(), v1, v1+3);
    for ( GLfloat attr : attributes2) { vec.insert(vec.end(), attr); }
    vec.insert(vec.end(), v3, v3+3);
    for ( GLfloat attr : attributes1) { vec.insert(vec.end(), attr); }
    //Bottom triangle
    vec.insert(vec.end(), v1, v1+3);
    for ( GLfloat attr : attributes2) { vec.insert(vec.end(), attr); }
    vec.insert(vec.end(), v2, v2+3);
    for ( GLfloat attr : attributes2) { vec.insert(vec.end(), attr); }
    vec.insert(vec.end(), v3, v3+3);
    for ( GLfloat attr : attributes1) { vec.insert(vec.end(), attr); }
}
vector<GLfloat> makeGlAttributePixelRow(GLuint length, GLuint y1, GLuint y2,
                                        const vector<GLfloat>& attributesTop,
                                        const vector<GLfloat>& attributesBottom)
{
    assert(attributesTop.size() == attributesBottom.size());
    //Layout:
    //x, y, z, t, {attributes}
    float v0[] = {0.0, static_cast<float>(y1), 0.5};//0
    float v1[] = {0.0, static_cast<float>(y2), 0.5};//1
    float v2[] = {static_cast<float>(length), static_cast<float>(y2), 0.5};//2
    float v3[] = {static_cast<float>(length), static_cast<float>(y1), 0.5};//3
    
    vector<GLfloat> result;
    //Top triangle
    result.reserve(6*4*attributesTop.size()*sizeof(GLfloat));
    result.insert(result.end(), v0, v0+3);
    result.insert(result.end(), attributesTop.begin(), attributesTop.end());
    result.insert(result.end(), v1, v1+3);
    result.insert(result.end(), attributesTop.begin(), attributesTop.end());
    result.insert(result.end(), v3, v3+3);
    result.insert(result.end(), attributesBottom.begin(), attributesBottom.end());
    //Bottom triangle
    result.insert(result.end(), v1, v1+3);
    result.insert(result.end(), attributesTop.begin(), attributesTop.end());
    result.insert(result.end(), v2, v2+3);
    result.insert(result.end(), attributesBottom.begin(), attributesBottom.end());
    result.insert(result.end(), v3, v3+3);
    result.insert(result.end(), attributesBottom.begin(), attributesBottom.end());
    return result;
}