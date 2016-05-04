//
//  engine.cpp
//  Delphinus
//
//  Created by Jos Kuijpers on 01/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#include "engine.hpp"
#include "macros.hpp"
#include "window.hpp"
#include "runtime.hpp"

#include <SDL2/SDL.h>

void printEvent(const SDL_Event *);

using namespace delphinus;

Engine::Engine() {
    runtime = new Runtime();

    // Load default scripts

    // SDL init
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        FATAL("SDL_Init error: %s\n", SDL_GetError());
    }

    window = new Window(800, 600);
}

Engine::~Engine() {
    delete window;
    delete runtime;

    SDL_Quit();
}

int Engine::run() {
    runtime->run();
/*
    bool running = true;

    while(running) {
        /// EVENTS START
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            printEvent(&event);

            if (event.type == SDL_QUIT) {
                printf("Quit!\n");
                running = false;
                break;
            }
        }
        /// EVENTS END
    }
*/

    return 0;
}

void printEvent(const SDL_Event *event) {
    switch (event->type) {
        case SDL_WINDOWEVENT: {
            switch (event->window.event) {
                case SDL_WINDOWEVENT_SHOWN:
                    printf("Window %d shown\n", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_HIDDEN:
                    printf("Window %d hidden\n", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_EXPOSED:
                    printf("Window %d exposed\n", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_MOVED:
                    printf("Window %d moved to (%d, %d)\n", event->window.windowID, event->window.data1, event->window.data2);
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    printf("Window %d resized to (%d, %d)\n", event->window.windowID, event->window.data1, event->window.data2);
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                    printf("Window %d minimized\n", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_MAXIMIZED:
                    printf("Window %d maximized\n", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_RESTORED:
                    printf("Window %d restored\n", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_ENTER:
                    printf("Window %d enter\n", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_LEAVE:
                    printf("Window %d leave\n", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    printf("Window %d focus gained\n", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    printf("Window %d focus lost\n", event->window.windowID);
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    printf("Window %d closed\n", event->window.windowID);
                    break;
                default:
                    printf("Unknown window event %d for window %d\n", event->window.event, event->window.windowID);
                    break;
            }
        }
            break;
        case SDL_QUIT:
            printf("Quit event\n");
            break;
        case SDL_KEYDOWN:
            printf("Key down\n");
            break;
        case SDL_KEYUP:
            printf("Key up\n");
            break;
        case SDL_MOUSEMOTION:
            printf("Mouse motion (%d,%d) v(%d,%d)\n", event->motion.x, event->motion.y, event->motion.xrel, event->motion.yrel);
            break;
        case SDL_MOUSEBUTTONDOWN:
            printf("Button %d down at (%d,%d)\n", event->button.button, event->button.x, event->button.y);
            break;
        case SDL_MOUSEBUTTONUP:
            printf("Button %d up at (%d,%d)\n", event->button.button, event->button.x, event->button.y);
            break;
        case SDL_MOUSEWHEEL:
            printf("Mouse wheel v(%d,%d)\n", event->wheel.x, event->wheel.y);
            break;
        case SDL_USEREVENT:
            printf("A userevent\n");
            break;
        case SDL_TEXTINPUT:
            printf("Text input '%s'\n", event->text.text);
            break;
        case SDL_TEXTEDITING:
            printf("Text editing '%s'  from %d for %d\n", event->edit.text, event->edit.start, event->edit.length);
            break;
        default:
            printf("Unknown event %d\n", event->type);
            break;
    }
}


    /*
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr){
        SDL_DestroyWindow(win);
        printf("SDL_CreateRenderer Error: %s",SDL_GetError());
        SDL_Quit();
        return 1;
    }*/

int main(int argc, const char * argv[]) {
    Engine engine;

    return engine.run();
}
