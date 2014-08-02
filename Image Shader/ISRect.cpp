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

#include "ISRect.h"

ISSize::ISSize() : _width(0), _height(0) { }
ISSize::ISSize(uint width, uint height) : _width(width), _height(height) { }
ISSize& ISSize::width(uint width) {
    _width = width;
    return *this;
}
ISSize& ISSize::height(uint height) {
    _height = height;
    return *this;
}
uint ISSize::width() const {
    return _width;
}
uint ISSize::height() const {
    return _height;
}
bool ISSize::operator==(const ISSize& rhs) const {
    return (_width == rhs._width) && (_height == rhs._height);
}
ISSize& ISSize::mult(uint k) {
    _width *= k;
    _height *= k;
    return *this;
}

ISRect::ISRect() : _left(0), _right(0), _top(0), _bottom(0) { }
ISRect::ISRect(uint left, uint right, uint top, uint bottom) : _left(left), _right(right), _top(top), _bottom(bottom) { }
ISRect::ISRect(ISSize size) : _left(0), _right(size.width()), _top(0), _bottom(size.height()) { }
ISRect& ISRect::left(uint left) {
    _left = left;
    return *this;
}
ISRect& ISRect::right(uint right) {
    _right = right;
    return *this;
}
ISRect& ISRect::top(uint top) {
    _top = top;
    return *this;
}
ISRect& ISRect::bottom(uint bottom) {
    _bottom = bottom;
    return *this;
}
uint ISRect::left() const {
    return _left;
}
uint ISRect::right() const {
    return _right;
}
uint ISRect::top() const {
    return _top;
}
uint ISRect::bottom() const {
    return _bottom;
}
uint ISRect::width() const {
    return _right - _left;
}
uint ISRect::height() const {
    return _bottom - _top;
}
ISSize ISRect::size() const {
    return ISSize(width(), height());
}
bool ISRect::operator==(const ISRect& rhs) const {
    return (_left == rhs._left) && (_right == rhs._right) && (_bottom == rhs._bottom) && (_top == rhs._top);
}