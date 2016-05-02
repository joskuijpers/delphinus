//
//  cleanup.hpp
//  Delphinus
//
//  Created by Jos Kuijpers on 01/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#pragma once

#include <utility>
#include <SDL2/SDL.h>

/*
 * Recurse through the list of arguments to clean up, cleaning up
 * the first one in the list each iteration.
 */
template<typename T, typename... Args>
void cleanup(T *t, Args&&... args){
    // Cleanup the first item in the list
    cleanup(t);
    // Recurse to clean up the remaining arguments
    cleanup(std::forward<Args>(args)...);
}

/*
 * These specializations serve to free the passed argument and also provide the
 * base cases for the recursive call above, eg. when args is only a single item
 * one of the specializations below will be called by
 * cleanup(std::forward<Args>(args)...), ending the recursion
 * We also make it safe to pass nullptrs to handle situations where we
 * don't want to bother finding out which values failed to load (and thus are null)
 * but rather just want to clean everything up and let cleanup sort it out
 */
template<>
inline void cleanup<SDL_Window>(SDL_Window *window){
    if (!window) {
        return;
    }
    SDL_DestroyWindow(window);
}

template<>
inline void cleanup<SDL_Renderer>(SDL_Renderer *renderer){
    if (!renderer) {
        return;
    }
    SDL_DestroyRenderer(renderer);
}

template<>
inline void cleanup<SDL_Texture>(SDL_Texture *texture){
    if (!texture) {
        return;
    }
    SDL_DestroyTexture(texture);
}

template<>
inline void cleanup<SDL_Surface>(SDL_Surface *surface){
    if (!surface) {
        return;
    }
    SDL_FreeSurface(surface);
}
