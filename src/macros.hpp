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

#ifdef DEBUG
# define LOG(format, ...) fprintf(stderr, format "\n", ##__VA_ARGS__);
#else
# define LOG(format, ...)
#endif

#define FATAL(format, ...) { fprintf(stderr, format "\n", ##__VA_ARGS__); exit(1); }