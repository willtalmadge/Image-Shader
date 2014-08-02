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

#include "ISPipelineBufferable.h"

CVEAGLContext ISPipelineBufferable::_context = NULL;

ISPipelineBufferable& ISPipelineBufferable::setTargetSize(uint width, uint height) {
    ISPipeline::setTargetSize(width, height);
    return *this;
}
ISPipelineBufferable& ISPipelineBufferable::setTargetSize(ISSize size) {
    ISPipeline::setTargetSize(size);
    return *this;
}
ISPipelineBufferable& ISPipelineBufferable::sourceToTargetSizeDiv(uint widthDiv, uint heightDiv) {
    ISPipeline::sourceToTargetSizeDiv(widthDiv, heightDiv);
    return *this;
}
ISPipelineBufferable& ISPipelineBufferable::sourceToTargetSizeMult(uint widthMult, uint heightMult) {
    ISPipeline::sourceToTargetSizeMult(widthMult, heightMult);
    return *this;
}
ISPipelineBufferable& ISPipelineBufferable::fromROI(ISRect roi) {
    ISPipeline::fromROI(roi);
    return *this;
}
ISPipelineBufferable& ISPipelineBufferable::toROI(ISRect roi) {
    ISPipeline::toROI(roi);
    return *this;
}
ISPipelineBufferable& ISPipelineBufferable::fullROI() {
    ISPipeline::fullROI();
    return *this;
}