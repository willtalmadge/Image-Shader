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

#ifndef Curve_Lab_Macro_h
#define Curve_Lab_Macro_h

#include <iostream>


extern size_t functionContextHash;

void tryPrintFunctionContext(const char* prettyName);

#ifdef DEBUG
#define DLPRINT(...) do{ tryPrintFunctionContext(__PRETTY_FUNCTION__); printf( __VA_ARGS__ ); } while( false )
#else
#define DLPRINT(...) do{ } while ( false )
#endif

#ifdef DEBUG
#define DPRINT(...) do{ printf( __VA_ARGS__ ); } while( false )
#else
#define DPRINT(...) do{ } while ( false )
#endif

#endif
