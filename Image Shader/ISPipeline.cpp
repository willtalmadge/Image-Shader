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

ISPipeline::ISPipeline(unique_ptr<ISTextureTuple> value) : _isRoot(false), _value(std::move(value)) {
    assert(_value);
    _value->split(1);
    _targetSize = _value->size();
    fullROI();
};
ISPipeline::ISPipeline(ISTextureTuple* value) : _isRoot(false), _value(unique_ptr<ISTextureTuple>(value)) {
    assert(_value);
    _targetSize = _value->size();
    fullROI();
};

void ISPipeline::releaseAllCaches()
{
    //Technically this is safe to call at any time, it is not recommended for performance reasons to call until the pipeline is finished
    ISTexture::releasePool();
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
void ISPipeline::defaultROI() {
    _sourceROI = _targetROI;
}
ISPipeline& ISPipeline::setTargetSize(uint width, uint height) {
    _targetSize.width(width).height(height);
    defaultROI();
    return *this;
}
ISPipeline& ISPipeline::setTargetSize(ISSize size) {
    _targetSize = size;
    defaultROI();
    return *this;
}
ISPipeline& ISPipeline::sourceToTargetSizeDiv(uint widthDiv, uint heightDiv) {
    ISSize size = _sourceROI.size();
    assert(size.width()%widthDiv == 0);
    assert(size.height()%heightDiv == 0);
    _targetSize.width(_sourceROI.size().width()/widthDiv)
               .height(_sourceROI.size().height()/heightDiv);
    _targetROI = ISRect(0, _targetSize.width(), 0, _targetSize.height());
    return *this;
}
ISPipeline& ISPipeline::sourceToTargetSizeMult(uint widthMult, uint heightMult) {
    //Multiplies source ROI dimensions and then sets the target ROI to use the whole target.
    _targetSize.width(_sourceROI.size().width()*widthMult)
               .height(_sourceROI.size().height()*heightMult);
    _targetROI = ISRect(0, _targetSize.width(), 0, _targetSize.height());
    return *this;
}
ISPipeline& ISPipeline::fromROI(ISRect roi) {
    _sourceROI = roi;
    return *this;
}
ISPipeline& ISPipeline::toROI(ISRect roi) {
    _targetROI = roi;
    return *this;
}
ISPipeline& ISPipeline::fullROI() {
    _sourceROI = ISRect(_value->size());
    _targetROI = ISRect(_targetSize);
    return *this;
}
ISSize ISPipeline::targetSize() const {
    return _targetSize;
}
ISRect ISPipeline::sourceROI() const {
    return _sourceROI;
}
ISRect ISPipeline::targetROI() const {
    return _targetROI;
}