//  Created by William Talmadge on 6/4/14.
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

#include "ISSingleton.h"

void ISSingleton::setup(std::unique_ptr<ISSingleton>& input)
{
    setTexture(input->getTexture());
}
ISTextureRef ISSingleton::getTexture() const
{
    assert(_elements.size() == 1);
    return _elements[0];
}
void ISSingleton::setTexture(ISTextureRef texture)
{
    _elements = {texture};
}
void ISSingleton::bind(const ISSingletonBindable* drawable) const
{
    ISTextureRef texture = getTexture();
    texture->bindToShader(drawable->textureBindingTarget(), 0); //FIXME: confusing semantics
}
void ISSingleton::attach() const { //TODO: Express the fact that only singleton is attachable really
    GLint attachedName;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &attachedName);
    if (attachedName != getTexture()->name()) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               getTexture()->name(), 0);
        DLPRINT("Attaching %d to framebuffer (type %d)\n", getTexture()->name(), getTexture()->type());
        glViewport(0, 0, getTexture()->width(), getTexture()->height());
    }
}
ISSize ISSingleton::size() const {
    auto size = ISSize().width(getTexture()->width())
                        .height(getTexture()->height());
    return size;
}