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

#include <jsapi.h>
#include <js/Initialization.h>

/* The error reporter callback. */
void reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
    fprintf(stderr, "%s:%u:%s\n",
            report->filename ? report->filename : "<no filename>",
            (unsigned int) report->lineno,
            message);
}

//https://nodejs.org/api/modules.html#modules_all_together

static JSClass global_class;

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

    SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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

    SDL_StartTextInput();

    while(running) {
        /// EVENTS START
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            printEvent(&event);

            if (event.type == SDL_QUIT) {
                std::cout << "Quit!" << std::endl;
                running = false;
                break;
            }
        }
        /// EVENTS END

        /// RENDER START
        SDL_RenderClear(ren);
            /// RENDER USER FUNC`
        SDL_RenderPresent(ren);
        /// RENDER END
    }

    SDL_Quit();

    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);
    JS_ShutDown();

    return 0;
}
