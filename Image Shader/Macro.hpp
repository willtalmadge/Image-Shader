//
//  Macro.hpp
//  Curve Lab
//
//  Created by William Talmadge on 9/22/13.
//  Copyright (c) 2013 William Talmadge. All rights reserved.
//

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
