//
//  ISTextureTuple.cpp
//  Image Shader
//
//  Created by William Talmadge on 6/1/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

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
ISTextureTuple::~ISTextureTuple()
{

}