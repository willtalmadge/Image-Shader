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

#include "ISTexture.h"
#include "ISSingleton.h"
#include "assert.h"
#include "Macro.hpp"

using namespace std;

GLenum ISTexture::_activeTextureUnit = GL_TEXTURE0;
std::set<ISTextureRef, ISTexture::poolCompare>
ISTexture::_texturePool = std::set<ISTextureRef, ISTexture::poolCompare>();

void ISTexture::releasePool()
{
    for (ISTextureRef texture : _texturePool)
    {
        texture->deleteTexture();
        delete texture;
    }
}
bool ISTexture::poolCompare::operator()(ISTextureRef lhs, ISTextureRef rhs) const
{
    //If the texture name of one of the textures is 0 then it is assumed
    //we need to find an existing texture, thus the name is ignored in the
    //comparision. Textures will compare as equivalent with different names
    //with everything else being equal.
    if (lhs->width() < rhs->width()) {
        return true;
    } else if (rhs->width() < lhs->width()) {
        return false;
    } else if (lhs->height() < rhs->height()) {
        return true;
    } else if (rhs->height() < lhs->height()) {
        return false;
    } else if (lhs->type() < rhs->type()) {
        return true;
    } else if (rhs->type() < lhs->type()) {
        return false;
    } else if (lhs->format() < rhs->format()) {
        return true;
    } else if (rhs->format() < lhs->format()) {
        return false;
    } else if ((rhs->_name!=0) & (lhs->_name!=0) & (rhs->_name < lhs->_name)) {
        return true;
    } else if ((rhs->_name!=0) & (lhs->_name!=0) & (rhs->_name > lhs->_name)) {
        return false;
    } else {
        return false;
    }
}
void ISTexture::attachToFramebuffer() const
{
    //There should only be one framebuffer for the entire pipeline.
    //ensureActiveTextureUnit(GL_TEXTURE0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _name, 0);
    glViewport(0, 0, _width, _height);
}
void ISTexture::attachToFramebuffer(GLuint framebuffer) const
{
    //To be used for non pipeline attachments;
    GLint existing;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &existing);
    if (existing != framebuffer) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _name, 0);
    glViewport(0, 0, _width, _height);
}
void ISTexture::detach() const
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
}
void ISTexture::ensureActiveTextureUnit(GLenum textureUnit) const
{
    assert(textureUnit >= GL_TEXTURE0);
    GLint activeTexture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    assert(activeTexture >= GL_TEXTURE0);
    if (textureUnit != activeTexture) {
        glActiveTexture(textureUnit);
        ISTexture::_activeTextureUnit = textureUnit;
    }
}
void ISTexture::bindToShader(GLuint shaderPosition, GLuint textureUnitOffset) const
{
    assert(textureUnitOffset < GL_TEXTURE0); //Just pass in the offset, as you see the base texture unit is added here
    ensureActiveTextureUnit(GL_TEXTURE0 + textureUnitOffset);
    DLPRINT("Binding %d to shader\n",_name);
    glBindTexture(GL_TEXTURE_2D, _name);
    glUniform1i(shaderPosition, textureUnitOffset);
}
void ISTexture::setup()
{
    auto texture = _texturePool.find(this);
    DLPRINT("Looking in texture cache (size %zu) for %dx%d (type %d)\n", _texturePool.size(),_width,_height,type());
    if (texture != _texturePool.end()) {
        assert((*texture)->isValid());
        DLPRINT("Cache hit for texture %d\n", (*texture)->name());
        _name = (*texture)->name();
        _texturePool.erase(texture);
        _isValid = true;
    }
    else {
        //make new texture
        DLPRINT("Cache miss, creating new texture\n");
        glGenTextures(1, &_name);
        ensureActiveTextureUnit(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _name);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, format(), _width, _height, 0, format(), type(), NULL);
        _isValid = true;
    }
}
void ISTexture::recycle() const
{
    DLPRINT("Caching texture %d\n", _name);

#ifdef DEBUG
    pair<set<ISTextureRef, poolCompare>::iterator, bool>
    result = _texturePool.insert(this);
    if (!result.second) {
        DLPRINT("Texture already in cache, no insertion made\n");
    }
#else
    _texturePool.insert(this);
#endif
}
void ISTexture::deleteTexture() const
{
    DLPRINT("Deleting texture %d\n", _name);
    glDeleteTextures(1, &_name);
}
ISSingleton* ISTexture::asSingleton() const
{
    ISSingleton* s = new ISSingleton();
    s->setTexture(this);
    return s;
}

void ISTexture::swap(ISTexture& other) throw() {
    using std::swap;
    swap(_width, other._width);
    swap(_height, other._height);
    swap(_name, other._name);
    swap(_isValid, other._isValid);
}
ISTexture::~ISTexture()
{
    //glDeleteTextures(1, &_name);
}
const ISURe8Rgba* ISURe8Rgba::fromExisting(GLuint name, GLuint width, GLuint height, GLenum type)
{
    assert(glIsTexture(name));
    assert(type == GL_UNSIGNED_BYTE);
    ISURe8Rgba* result = new ISURe8Rgba(width, height);
    result->_name = name;
    result->_isValid = true;
    return result;
}
const ISRe16Rgba* ISRe16Rgba::fromExisting(GLuint name, GLuint width, GLuint height, GLenum type)
{
    assert(glIsTexture(name));
    assert(type == GL_HALF_FLOAT_OES);
    ISRe16Rgba* result = new ISRe16Rgba(width, height);
    result->_name = name;
    result->_isValid = true;
    return result;
}
