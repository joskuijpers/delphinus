//
//  runtime.cpp
//  Delphinus
//
//  Created by Jos Kuijpers on 01/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#include "runtime.hpp"
#include "macros.hpp"
#include "module.hpp"

#include <jsapi.h>
#include <js/Initialization.h>

#include <SDL2/SDL_filesystem.h>

/// Error reporting callback for SpiderMonkey.
static void reportError(JSContext *cx, const char *message, JSErrorReport *report);

delphinus::Runtime::Runtime() {
    // Initialize the JS engine
    JS_Init();

    // Create a runtime
    runtime = JS_NewRuntime(8L * 1024L * 1024L);
    if (runtime == nullptr)
        FATAL("Could not create JS runtime");

    JS_SetErrorReporter(runtime, reportError);

    // Can do without management. JS runtime quits before Runtime is deallocated
    JS_SetRuntimePrivate(runtime, (void *)this);

    // Create a context
    context = JS_NewContext(runtime, 8192);
    if (context == nullptr)
        FATAL("Could not create JS context");
}

delphinus::Runtime::~Runtime() {
    JS_DestroyContextNoGC(context);
    JS_DestroyRuntime(runtime);

    JS_ShutDown();
}

delphinus::Runtime *delphinus::Runtime::getCurrent(JSContext *context) {
    return (delphinus::Runtime *)JS_GetRuntimePrivate(JS_GetRuntime(context));
}

void delphinus::Runtime::run() {
    // Load main module
    Module *mainModule = new Module(this, "main", std::string(SDL_GetBasePath()) + "main.js");

    mainModule->loadIntoRuntime(this);
}

static void reportError(JSContext *cx, const char *message, JSErrorReport *report) {
    const char *filename = report->filename ? report->filename : "<no filename>";
    const char *errorType = "Error";

    if (JSREPORT_IS_STRICT_MODE_ERROR(report->flags)) {
        errorType = "StrictError";
    } else if (JSREPORT_IS_WARNING(report->flags)) {
        errorType = "Warning";
    } else if (JSREPORT_IS_EXCEPTION(report->flags)) {
        errorType = "Exception";
    }

    LOG("%s in %s:%u: %s", errorType, filename, (uint)report->lineno, message);
}
