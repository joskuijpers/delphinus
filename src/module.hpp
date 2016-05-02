//
//  module.hpp
//  Delphinus
//
//  Created by Jos Kuijpers on 01/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#pragma once

#include <string>
#include <jsapi.h>
#include "types.hpp"

namespace delphinus {

    class Runtime;

class Module {
    std::string name;

    JS::Heap<JSObject *> exports;
    JS::Heap<JSObject *> scope;
    JS::Heap<JSScript *> script;

    std::string _filename;
    std::string _directory;

public:
    Module(Runtime *runtime, std::string moduleId, std::string path);

    bool loadIntoRuntime(Runtime *runtime);

    JSObject *getExports(JSContext *context);
private:
    static void module_trace_func(JSTracer *tracer, void *data);
    std::string getScriptPath();
    bool addGlobals(JSContext *context, JS::HandleObject moduleScope);
};
    
}