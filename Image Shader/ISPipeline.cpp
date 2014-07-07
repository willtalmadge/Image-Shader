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

#include "ISPipeline.h"
#include "ISTextureTuple.h"
#include "ISDrawable.h"

using namespace std;

GLuint ISPipeline::_framebufferName = 0;
bool ISPipeline::_rootInitialized = false;
void ISPipeline::setupRoot()
{
    assert(!_rootInitialized); //Don't use two root ISPipelines at once!
    //Call this when creating a root of an image shader pipeline.
    glGenFramebuffers(1, &_framebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebufferName);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    _isRoot = true;
    _rootInitialized = true;
}
void ISPipeline::teardown()
{
    glDeleteFramebuffers(1, &_framebufferName);
    _rootInitialized = false;
}

ISPipeline::ISPipeline(unique_ptr<ISTextureTuple> value) : _isRoot(false), _value(std::move(value)) { };
ISPipeline::ISPipeline(ISTextureTuple* value) : _isRoot(false), _value(unique_ptr<ISTextureTuple>(value)) { };

void ISPipeline::releaseAllCaches()
{
    //Technically this is safe to call at any time, it is not recommended for performance reasons to call until the pipeline is finished
    ISTexture::releaseCache();
    ISDrawableCache::releaseCache();
}

ISPipeline::~ISPipeline()
{
    if (_isRoot) {
        _value->deleteTextures();
        _rootInitialized = false;
    }
    _value.release(); //TODO: make sure this isn't causing any leaks
}