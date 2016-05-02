//
//  window.cpp
//  Delphinus
//
//  Created by Jos Kuijpers on 01/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#include "window.hpp"
#include "cleanup.hpp"
#include "macros.hpp"

#include <SDL2/SDL.h>

delphinus::Window::Window(uint width, uint height) {
    const char *title = "Hello";
    uint options = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              width, height, options);

    if (window == nullptr)
        FATAL("SDL_CreateWindow Error: %s", SDL_GetError());
}

delphinus::Window::~Window() {
    printf("Destroy window");
    cleanup(window);
}

void delphinus::Window::show() {
    SDL_ShowWindow(window);
}

void delphinus::Window::hide() {
    SDL_HideWindow(window);
}

uint delphinus::Window::getWidth() {
    int width;

    SDL_GetWindowSize(window, &width, nullptr);

    return width;
}

uint delphinus::Window::getHeight() {
    int height;

    SDL_GetWindowSize(window, nullptr, &height);

    return height;
}