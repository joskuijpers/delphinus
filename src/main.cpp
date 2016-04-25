//
//  main.cpp
//  Delphinus
//
//  Created by Jos Kuijpers on 25/04/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#include <iostream>
#include <memory.h>

#include <SDL2/SDL.h>
#include "jsapi.hpp"

/* The error reporter callback. */
void reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
    fprintf(stderr, "%s:%u:%s\n",
            report->filename ? report->filename : "<no filename>",
            (unsigned int) report->lineno,
            message);
}

static JSClass global_class;

int main(int argc, const char * argv[]) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
        return 1;
    }

    std::cout << "Hello" << std::endl;

    memset(&global_class, 0, sizeof(JSClass));
    global_class.name = "global";
    global_class.flags = JSCLASS_GLOBAL_FLAGS;
    global_class.trace = JS_GlobalObjectTraceHook;


    JS_Init();

    JSRuntime *rt = JS_NewRuntime(8L * 1024L * 1024L);
    if (rt == nullptr)
        return 1;

    JS_SetErrorReporter(rt, reportError);

    JSContext *cx = JS_NewContext(rt, 8192);
    if (cx == nullptr)
        return 1;

    {
        JSAutoRequest ar(cx); // exit this any time spinning event loop?

        JS::RootedObject global(cx, JS_NewGlobalObject(cx, &global_class, nullptr, JS::OnNewGlobalHookOption::FireOnNewGlobalHook, JS::CompartmentOptions()));
        if (!global)
            return 1;

        JS::RootedValue rval(cx);

        {
            JSAutoCompartment ac(cx, global);

            if (!JS_InitStandardClasses(cx, global))
                return 1;

            const char *script = "function foo() { return 'hello'+'world, it is '+new Date(); }; foo();";
            const char *filename = "noname";
            int lineno = 1;

            JS::OwningCompileOptions options(cx);
            options.setFileAndLine(cx, filename, lineno);
            options.setIntroductionScript(nullptr);
            options.setVersion(JSVERSION_LATEST);
            options.setUTF8(true);
            options.asmJSOption = JS::AsmJSOption::Enabled;
            options.strictOption = true;

            JS::RootedScript jsScript(cx);

            if (!JS::Compile(cx, options, script, strlen(script), &jsScript)) {
                std::cerr << "Cant compile script" << std::endl;
                return 1;
            }

            JS_MaybeGC(cx);

            if (!JS_ExecuteScript(cx, jsScript, &rval)) {
                std::cerr << "Cant eval script" << std::endl;
                return 1;
            }
        }

        JSString *str = rval.toString();
        printf("%s\n", JS_EncodeString(cx, str));
    }

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

    bool running = true;
    unsigned int num = 0;

    while(running) {
        /// EVENTS START
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            std::cout << "Got event " << num++ << ": " << event.type << std::endl;

            if (event.type == SDL_QUIT) {
                std::cout << "Quit!" << std::endl;
                running = false;
                break;
            }
        }
        /// EVENTS END

        /// RENDER START
        SDL_RenderClear(ren);
            /// RENDER USER FUNC
        SDL_RenderPresent(ren);
        /// RENDER END
    }

    SDL_Quit();

    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);
    JS_ShutDown();

    return 0;
}
