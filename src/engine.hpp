//
//  engine.hpp
//  Delphinus
//
//  Created by Jos Kuijpers on 01/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#pragma once

#include <memory>

#include "window.hpp"
#include "runtime.hpp"

namespace delphinus {

class Engine : public std::enable_shared_from_this<Engine> {
    Window *window;
    Runtime *runtime;

public:
    Engine();
    ~Engine();

    /**
     * Run the engine.
     */
    int run();

private:
};
    
}
