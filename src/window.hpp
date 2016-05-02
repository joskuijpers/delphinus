//
//  window.hpp
//  Delphinus
//
//  Created by Jos Kuijpers on 01/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#pragma once

#include <memory>
#include <SDL2/SDL.h>
#include "types.hpp"

namespace delphinus {

class Window : public std::enable_shared_from_this<Window> {
    SDL_Window *window;
//    SDL_GLContext *glContext;

public:
    Window(uint width, uint height);
    ~Window();

    void show();
    void hide();

    uint getWidth();
    uint getHeight();
private:
};

}