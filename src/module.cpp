//
//  module.cpp
//  Delphinus
//
//  Created by Jos Kuijpers on 01/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#include "module.hpp"
#include "macros.hpp"
#include "runtime.hpp"
#include "util.hpp"
#include "console_api.hpp"

#include <cassert>

#include <SDL2/SDL_filesystem.h>
#include <js/Conversions.h>

bool api_require(JSContext *context, uint argc, JS::Value *vp);
bool api_resolve(JSContext *context, uint argc, JS::Value *vp);

static JSClass moduleScopeClass = {
    "global", JSCLASS_GLOBAL_FLAGS,
    .trace =  JS_GlobalObjectTraceHook
};

static JSClass moduleClass = {
    "Module", 0
};

static bool ModuleObject(JSContext *context, uint argc, JS::Value *vp) {
    return true;
}

#pragma mark - Module Class

delphinus::Module::Module(Runtime *runtime, std::string moduleId, std::string path) {
    size_t splitPos = path.rfind("/");
    if (splitPos == std::string::npos) {
        _filename = path;
        _directory = "";
    } else {
        _filename = path.substr(splitPos + 1);
        _directory = path.substr(0, splitPos);
    }

    name = path; // TODO make unique

    JS_AddExtraGCRootsTracer(runtime->runtime, module_trace_func, this);
}

// TODO: make an actual safe method... this sucks.
// Resolve to having to ..
std::string delphinus::Module::getScriptPath() {
    return _directory + "/" + _filename;
}

bool delphinus::Module::addGlobals(JSContext *context, JS::HandleObject moduleScope) {
    // Add std classes
    if (!JS_InitStandardClasses(context, moduleScope)) {
        LOG("Could not create standard classes");
        return false;
    }

    // Add console object
    if (!delphinus::api::console_addToScope(context, moduleScope)) {
        LOG("Failed to add console to module scope");
        return false;
    }


    // Add __delphinus object

    return true;
}

bool delphinus::Module::loadIntoRuntime(Runtime *runtime) {
    JSContext *context = runtime->context;

    assert(!JS_IsExceptionPending(context));

#pragma mark - Create scope

    JSAutoRequest ar(context);

    // Create global
    JS::RootedObject moduleScope(context, JS_NewGlobalObject(context, &moduleScopeClass, nullptr, JS::FireOnNewGlobalHook));
    if (!moduleScope) {
        LOG("Could not create global object");
        return false;
    }
    scope = moduleScope;

#pragma mark - Fill scope

    JSAutoCompartment ac(context, moduleScope);

    addGlobals(context, moduleScope);

    // Add __dirname and __filename properties
    JS::RootedString __dirname(context, JS_NewStringCopyZ(context, _directory.c_str()));
    if(!JS_DefineProperty(context, moduleScope,
                          "__dirname", __dirname,
                          JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT)) {
        LOG("Failed to add __dirname property to module scope");
        return false;
    }

    JS::RootedString __filename(context, JS_NewStringCopyZ(context, _filename.c_str()));
    if (!JS_DefineProperty(context, moduleScope,
                           "__filename", __filename,
                           JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT)) {
        LOG("Failed to add __filename property to module scope");
        return false;
    }

    // Define require function
    JS::RootedObject require(context, (JSObject *)JS_DefineFunction(context,
                                                                    moduleScope,
                                                                    "require",
                                                                    &api_require, 1,
                                                                    JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT));
    if (!require) {
        LOG("Failed to define 'require'");
        return false;
    }

    if(!JS_DefineFunction(context, require,
                          "resolve", &api_resolve, 1,
                          JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT)) {
        LOG("Failed to define 'require.resolve'");
        return false;
    }

    // Add require.main to tell what the main module is.
    JS::RootedValue mainModule(context, JS::NullValue()); // TODO
    if (!JS_DefineProperty(context, require,
                           "main", mainModule,
                           JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT)) {
        LOG("Failed to define 'require.main'");
        return false;
    }

    //    JS_SetPrivate(require.get(), (void *)this);

    // Add exports and module properties
    exports = JS_NewPlainObject(context);
    JS::RootedObject rtExports(context, exports);
    if (!JS_DefineProperty(context, moduleScope,
                           "exports", rtExports,
                           JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT)) {
        LOG("Failed to define 'exports");
        return false;
    }

    // Create the Module class and prototype
    JS::RootedObject moduleObjectProto(context, JS_InitClass(context, moduleScope, nullptr, &moduleClass, ModuleObject, 0, nullptr, nullptr, nullptr, nullptr));
    if (!moduleObjectProto) {
        FATAL("Could not create Module prototype");
    }

    // Create a new module object
    JS::RootedObject moduleObj(context, JS_NewObjectWithGivenProto(context, &moduleClass, moduleObjectProto));
    if (!JS_DefineProperty(context, moduleScope,
                           "module", moduleObj,
                           JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT)) {
        LOG("Failed to define 'module");
        return false;
    }

    // Add 'module.id'
    JS::RootedString moduleId(context, JS_NewStringCopyZ(context, name.c_str()));
    if (!JS_DefineProperty(context, moduleObj,
                           "id", moduleId,
                           JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT)) {
        LOG("Failed to define 'module.id'");
        return false;
    }

#pragma mark - Load and run script
    // Load script data from
    std::string path = getScriptPath();
    std::string scriptContents;
    if (!readFile(path, scriptContents))
        return false;

    JS::OwningCompileOptions options(context);
    options.setFileAndLine(context, name.c_str(), 1);
    options.setIntroductionScript(nullptr); // script that initiated the require()
    options.setVersion(JSVERSION_LATEST);
    options.setUTF8(true);
    options.asmJSOption = JS::AsmJSOption::Enabled;
    options.strictOption = true;

    JS::RootedScript rtScript(context);
    if (!JS::Compile(context, options, scriptContents.c_str(),
                     strlen(scriptContents.c_str()), &rtScript)) {
        return false;
    }
    script = rtScript;

    JS::RootedValue rval(context);
    return JS_ExecuteScript(context, rtScript, &rval);
}

#pragma mark - Memory management

void delphinus::Module::module_trace_func(JSTracer *tracer, void *data) {
    Module *mod = (Module *)data;

    if (mod->exports) JS_CallObjectTracer(tracer, &(mod->exports), "exports");
    if (mod->scope) JS_CallObjectTracer(tracer, &(mod->scope), "scope");
    if (mod->script) JS_CallScriptTracer(tracer, &(mod->script), "script");
}

JSObject *delphinus::Module::getExports(JSContext *context) {
    JS::RootedObject rtExports(context, exports);

    return rtExports;
}

#pragma mark - API

std::string resolveModuleId(std::string moduleId) {
    return std::string(SDL_GetBasePath()) + "/test.js";
}

bool api_resolve(JSContext *context, uint argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    std::string module = stringFromValue(context, args.get(0));
    std::string path = resolveModuleId(module);

    args.rval().setString(JS_NewStringCopyZ(context, path.c_str()));
    return true;
}

bool api_require(JSContext *context, uint argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    std::string moduleId = stringFromValue(context, args.get(0));
    std::string path = resolveModuleId(moduleId);

    LOG("require('%s') called. Resolved to %s", moduleId.c_str(), path.c_str());

    // Get the runtime
    delphinus::Runtime *runtime = delphinus::Runtime::getCurrent(context);

    // Create the module
    // TODO; actually get it from cache
    delphinus::Module *module = new delphinus::Module(runtime, moduleId, path);

    // Load the module
    if (!module->loadIntoRuntime(runtime))
        return false;

    // Get the exports
    JS::RootedObject exports(context, module->getExports(context));
    if (!JS_WrapObject(context, &exports)) {
        args.rval().setUndefined();
    } else {
        args.rval().setObjectOrNull(exports);
    }

    return true;
}
