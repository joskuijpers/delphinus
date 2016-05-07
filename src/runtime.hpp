//
//  runtime.hpp
//  Delphinus
//
//  Created by Jos Kuijpers on 01/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#pragma once

#include <memory>

#include "types.hpp"
#include <jsapi.h>

#include "spherefs.hpp"

namespace delphinus {

class Runtime : public std::enable_shared_from_this<Runtime> {
public:
    JSRuntime *runtime;
    JSContext *context;

    Sandbox *sandbox;
public:
    Runtime();
    ~Runtime();

    void run();

    inline Sandbox *getSandbox() {
        return sandbox;
    }

    static Runtime *getCurrent(JSContext *context);
};

}