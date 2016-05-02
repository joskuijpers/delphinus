//
//  console_api.cpp
//  Delphinus
//
// The console is a global object so we can't handle it in a
// javascript module.
//
//  Created by Jos Kuijpers on 02/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#include "console_api.hpp"
#include "types.hpp"
#include "util.hpp"
#include "macros.hpp"

#include <js/Conversions.h>

#include <cstring>
#include <cstdio>
#include <string>

bool api_new_console(JSContext *context, uint argc, JS::Value *vp) {
    return true;
}

bool api_console_print(JSContext *context, uint argc, JS::Value *vp, const char *prefix) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    // Concatenate all arguments
    std::string result = "";
    for(uint i = 0; i < args.length(); ++i) {
        JS::RootedString jsString(context, args.get(i).toString());

        char *moduleName = JS_EncodeStringToUTF8(context, jsString);
        if(moduleName) {
            result.append(" ");
            result.append(moduleName);

            JS_free(context, moduleName);
        }
    }

    printf("[CONSOLE: %s] %s\n", prefix, result.c_str());
    
    return true;
}

bool api_console_log(JSContext *context, uint argc, JS::Value *vp) {
    return api_console_print(context, argc, vp, "  LOG");
}

bool api_console_warn(JSContext *context, uint argc, JS::Value *vp) {
    return api_console_print(context, argc, vp, " WARN");
}

bool api_console_error(JSContext *context, uint argc, JS::Value *vp) {
    return api_console_print(context, argc, vp, "ERROR");
}

// .dir
// .trace
// .group
// .unGroup
// .time
// .timeEnd

static const JSFunctionSpec consoleFunctions[] = {
    JS_FS("log", api_console_log, 1, 0),
    JS_FS("warn", api_console_warn, 1, 0),
    JS_FS("error", api_console_error, 1, 0),
    JS_FS_END
};

bool delphinus::api::console_addToScope(JSContext *context, JS::HandleObject scope) {
    static JSClass consoleClass = {
        "Console", 0
    };

    // Create prototype
    JS::RootedObject prototype(context, JS_InitClass(context, scope, nullptr, &consoleClass, &api_new_console, 0, nullptr, nullptr, nullptr, nullptr));
    if (!prototype) {
        return false;
    }

    // Add methods to the prototype
    if (!JS_DefineFunctions(context, prototype, consoleFunctions)) {
        return false;
    }

    // Create instance
    JS::RootedObject console(context, JS_NewObjectWithGivenProto(context, &consoleClass, prototype));
    if (!console) {
        return false;
    }

    // Add console to the scope
    if (!JS_DefineProperty(context, scope, "console", console, JSPROP_ENREPE)) {
        return false;
    }

    return true;
}
