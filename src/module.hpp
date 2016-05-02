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
    const char *name;

    JS::Heap<JSObject *> exports;
    JS::Heap<JSObject *> scope;
    JS::Heap<JSScript *> script;

    std::string _filename;
    std::string _directory;

public:
    Module(Runtime *runtime, const std::string &directory, const std::string &scriptName);
    ~Module();

    bool run(Runtime *runtime);

    static void module_trace_func(JSTracer *tracer, void *data);
private:
    std::string getScriptPath();
};
    
}