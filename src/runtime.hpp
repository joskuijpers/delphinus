//
//  runtime.hpp
//  Delphinus
//
//  Created by Jos Kuijpers on 01/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#pragma once

#include "types.hpp"
#include <jsapi.h>

namespace delphinus {

class Runtime {
public:
    JSRuntime *runtime;
    JSContext *context;

public:
    Runtime();
    ~Runtime();

    void run();
};

}