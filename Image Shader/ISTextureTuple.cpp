//  Created by William Talmadge on 6/1/14.
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

#include "ISTextureTuple.h"
#include <algorithm>
#include <iterator>
#include "ISPipeline.h"

using namespace std;

void ISTextureTuple::join(ISTextureTuple *input)
{
    vector<ISTextureRef> difference;
    for (ISTextureRef inTex : input->_elements) {
        bool inOutput = false;
        for (ISTextureRef outTex : _elements) {
            if (inTex->name() == outTex->name()) {
                inOutput = true;
                break;
            }
        }
        if (!inOutput) {
            difference.push_back(inTex);
        }
    }
    for (ISTextureRef oldTexture : difference) {
        oldTexture->recycle();
    }
}
void ISTextureTuple::deleteTextures() {
    for (ISTextureRef texture : _elements) {
        texture->deleteTexture();
    }
}
ISPipeline ISTextureTuple::pipeline()
{
    ISPipeline result(this);
    return result;
}
void ISTextureTuple::map(std::function<void (ISTextureRef)> f)
{
    for (ISTextureRef texture : _elements) {
        f(texture);
    }
}
void ISTextureTuple::split(size_t n)
{
    map([=](ISTextureRef tex) {
        tex->split(n);
    });
}
void ISTextureTuple::glue()
{
    map([](ISTextureRef tex) {
        tex->glue();
    });
}
void ISTextureTuple::terminate()
{
    map([](ISTextureRef tex) {
        tex->terminate();
    });
}
ISTextureTuple::~ISTextureTuple()
{

}