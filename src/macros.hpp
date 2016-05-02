//
//  macros.hpp
//  Delphinus
//
//  Created by Jos Kuijpers on 01/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#pragma once

#include "config.hpp"
#include <stdio.h>
#include <jsapi.h>

#ifdef DEBUG
# define LOG(format, ...) fprintf(stderr, format "\n", ##__VA_ARGS__);
#else
# define LOG(format, ...)
#endif

#define FATAL(format, ...) { fprintf(stderr, format "\n", ##__VA_ARGS__); exit(1); }

#define DPH_JS_STRING(name, value) JS::RootedString name(context, JS_NewStringCopyZ(context, value));
#define JSPROP_ENREPE (JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT)