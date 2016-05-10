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

using namespace delphinus;

/// Error reporting callback for SpiderMonkey.
static void reportError(JSContext *cx, const char *message, JSErrorReport *report);

Runtime::Runtime() {
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

    moduleCache_create();

    sandbox = new Sandbox();
}

Runtime::~Runtime() {
    delete sandbox;
    moduleCache_dispose();

    JS_DestroyContextNoGC(context);
    JS_DestroyRuntime(runtime);

    JS_ShutDown();
}

Runtime *Runtime::getCurrent(JSContext *context) {
    return (Runtime *)JS_GetRuntimePrivate(JS_GetRuntime(context));
}

void Runtime::run() {
    /*std::vector<Path> paths;

    paths.push_back(Path("tools.js"));
    paths.push_back(Path("/tools.js"));
    paths.push_back(Path("a//tools.js"));

    paths.push_back(Path("./tools.js"));
    paths.push_back(Path("hello/tools.js"));
    paths.push_back(Path("~usr/savefile.json"));
    paths.push_back(Path("~sys/scripts/fiel"));
    paths.push_back(Path("packages/link/link.js"));
    paths.push_back(Path("packages/link/../link/link.js"));
    paths.push_back(Path("../link/link.js"));
    paths.push_back(Path("link/./lala.js"));

    paths.push_back(Path("~sys/../test.js"));

    paths.push_back(Path("scripts/hello", "./file.js"));
    paths.push_back(Path("scripts/hello", "../file/file.js"));

    for (size_t i = 0; i < paths.size(); ++i) {
        Path p = paths[i];
        if (p.isValid()) {
            LOG("FSPath %s => %s", p.getString().c_str(), sandbox->resolve(p).c_str());
        }
    }*/

    // Load main module
    Module *mainModule = new Module(this, "main.js");

    mainModule->loadIntoRuntime();

//    runtime_require(this, "main");

    delete mainModule;
}

static bool printStackTrace(JSContext *context, JS::HandleObject stracktrace) {

    return true;
}

static void reportError(JSContext *context, const char *message, JSErrorReport *report) {
    const char *filename = report->filename ? report->filename : "<no filename>";
    const char *errorType = "Error";

    if (!report) {
        fprintf(stderr, "JS error from unknown source: %s\n", message);
    }

    if (JSREPORT_IS_STRICT_MODE_ERROR(report->flags)) {
        errorType = "StrictError";
    } else if (JSREPORT_IS_WARNING(report->flags)) {
        errorType = "Warning";
    } else if (JSREPORT_IS_EXCEPTION(report->flags)) {
        errorType = "Exception";
    }

    fprintf(stderr, "%s in %s:%u:%u %s\n", errorType, filename, (uint)report->lineno, (uint)report->column, message);

    if (JS_IsExceptionPending(context)) {
        JS::RootedValue excValue(context);

        JS_GetPendingException(context, &excValue); 

        if (!excValue.isUndefined()) {
            JS::RootedObject excObject(context);
            if (!JS_ValueToObject(context, excValue, &excObject)) {
                return;
            }

            LOG("Get stacktrace");
            JS::RootedObject stacktrace(context, ExceptionStackOrNull(context, excObject));
            printStackTrace(context, stacktrace);
        }
    }
}
