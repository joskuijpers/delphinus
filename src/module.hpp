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
#include "spherefs.hpp"

namespace delphinus {

class Runtime;

class Module : public std::enable_shared_from_this<Module> {
    std::string name; // Also normalized SFS path

    JS::Heap<JSObject *> exports;
    JS::Heap<JSObject *> scope;
    JS::Heap<JSScript *> script;

    Runtime *runtime;

public:
    Module(Runtime *runt, std::string moduleId);
    ~Module();

    bool loadIntoRuntime();

    JSObject *getExports(JSContext *context);
    std::string getModuleId();
private:
    static void module_trace_func(JSTracer *tracer, void *data);
    bool addGlobals(JSContext *context, JS::HandleObject moduleScope);
};

class ModuleCache {
    std::forward_list<std::shared_ptr<Module>> list;

public:
    ModuleCache() {};
    ~ModuleCache();

    std::shared_ptr<Module> lookup(std::string moduleId);
    void add(std::shared_ptr<Module> module);
};

class Package {
    Path packagePath;
    std::string name;
    std::string main;
    bool loaded;

public:
    Package();

    /// Use with path of the package, not the package.json
    bool load(Runtime *runtime, Path path);

    inline std::string getName() {
        return name;
    }

    inline std::string getMain() {
        return main;
    }

    inline Path getMainPath() {
        return Path(packagePath, main);
    }
};

void moduleCache_create();
void moduleCache_dispose();

/// Require from the runtime, to load system modules. moduleId must be normalized
JSObject *runtime_require(Runtime *runtime, std::string moduleId);

}