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
#include <cstring>
#include <cstdio>

namespace delphinus {
namespace api {

bool console_log(JSContext *context, uint argc, JS::Value *vp) {
    printf("log something");

    return false;
}

static JSFunctionSpec consoleFunctions[] = {
    JS_FS("log", console_log, 1, 0),
    JS_FS_END
};

Console::Console() {
    memset(&_class, 0, sizeof(JSClass));
    _class.name = "Console";
}

}
}

// .log
// .error
// .warn
// .dir
// .trace
// .group
// .unGroup
// .time
// .timeEnd