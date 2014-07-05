//
//  Macro.cpp
//  Curve Lab
//
//  Created by William Talmadge on 9/22/13.
//  Copyright (c) 2013 William Talmadge. All rights reserved.
//

#include "Macro.hpp"
#include <functional>

size_t functionContextHash = 0;

void tryPrintFunctionContext(const char* prettyName)
{
    std::hash<const char*> hasher;
    size_t hash = hasher(prettyName);
    if (hash != functionContextHash)
    {
        functionContextHash = hash;
        printf("\n==== %s ====\n", prettyName);
    }
}