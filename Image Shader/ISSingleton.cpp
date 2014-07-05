//
//  ISSingleton.cpp
//  Image Shader
//
//  Created by William Talmadge on 6/4/14.
//  Copyright (c) 2014 William Talmadge. All rights reserved.
//

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