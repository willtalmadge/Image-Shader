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

#ifndef __Image_Shader__ISRect__
#define __Image_Shader__ISRect__

typedef unsigned int uint;

struct ISSize {
    ISSize();
    ISSize(uint width, uint height);
    ISSize& width(uint width);
    ISSize& height(uint height);
    uint width() const;
    uint height() const;
    ISSize& mult(uint k);
    bool operator==(const ISSize& rhs) const;
protected:
    uint _width;
    uint _height;
};

struct ISRect {
    ISRect();
    ISRect(uint left, uint right, uint top, uint bottom);
    ISRect(ISSize size);
    ISRect& left(uint left);
    ISRect& right(uint right);
    ISRect& top(uint top);
    ISRect& bottom(uint bottom);
    uint left() const;
    uint right() const;
    uint top() const;
    uint bottom() const;
    uint width() const;
    uint height() const;
    ISSize size() const;
    bool operator==(const ISRect& rhs) const;
protected:
    uint _left;
    uint _right;
    uint _top;
    uint _bottom;
};

#endif /* defined(__Image_Shader__ISRect__) */
