//
//  main.cpp
//  Delphinus
//
//  Created by Jos Kuijpers on 25/04/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#include <iostream>
#include <SDL2/SDL.h>

int main(int argc, const char * argv[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
        return 1;
    }

    std::cout << "Hello" << std::endl;

    SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (win == nullptr){
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr){
        SDL_DestroyWindow(win);
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    while(true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            std::cout << "Got event" << std::endl;
        }

        //First clear the renderer
        SDL_RenderClear(ren);
        //Draw the texture
//        SDL_RenderCopy(ren, tex, NULL, NULL);
        //Update the screen
        SDL_RenderPresent(ren);

        //Take a quick break after all that hard work
        SDL_Delay(1000);
    }

    SDL_Quit();

    return 0;
}
