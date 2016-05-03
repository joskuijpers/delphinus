//
//  module.hpp
//  Delphinus
//
//  Created by Jos Kuijpers on 01/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#pragma once

#include <string>
#include <memory>
#include <forward_list>

#include <jsapi.h>
#include "types.hpp"

namespace delphinus {

class Runtime;

class Module : public std::enable_shared_from_this<Module> {
    std::string name;

    JS::Heap<JSObject *> exports;
    JS::Heap<JSObject *> scope;
    JS::Heap<JSScript *> script;

    std::string _filename;
    std::string _directory;

    Runtime *runtime;

public:
    Module(Runtime *runt, std::string moduleId, std::string path);
    ~Module();

    bool loadIntoRuntime();

    JSObject *getExports(JSContext *context);
    std::string getModuleId();
private:
    static void module_trace_func(JSTracer *tracer, void *data);
    std::string getScriptPath();
    bool addGlobals(JSContext *context, JS::HandleObject moduleScope);
};

class ModuleCache {
    std::forward_list<std::shared_ptr<Module>> list;

public:
    ModuleCache();
    ~ModuleCache();

    std::shared_ptr<Module> lookup(std::string moduleId);
    void add(std::shared_ptr<Module> module);
};

void moduleCache_create();
void moduleCache_dispose();

/// Require from the runtime, to load system modules
JSObject *runtime_require(Runtime *runtime, std::string moduleId);

}