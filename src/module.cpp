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

#include <fstream>
#include <sstream>

#include <SDL2/SDL_filesystem.h>

#include <js/Conversions.h>

namespace delphinus {

std::string readFile(std::string path);
std::string stringFromValue(JSContext *context, JS::HandleValue value);
bool api_require(JSContext *context, uint argc, JS::Value *vp);

static JSClass moduleScopeClass = {
    "global", JSCLASS_GLOBAL_FLAGS,
    .trace =  JS_GlobalObjectTraceHook
};

static JSClass moduleClass = {
    "Module", 0
};

void Module::module_trace_func(JSTracer *tracer, void *data) {
    Module *mod = (Module *)data;

    if (mod->exports)
        JS_CallObjectTracer(tracer, &(mod->exports), "exports");

    if (mod->scope)
        JS_CallObjectTracer(tracer, &(mod->scope), "scope");

    if (mod->script)
        JS_CallScriptTracer(tracer, &(mod->script), "script");
}

Module::Module(Runtime *runtime, const std::string &directory, const std::string &scriptName) {
    _filename = scriptName;
    _directory = directory;

    name = "hello"; // TODO make unique

    JS_AddExtraGCRootsTracer(runtime->runtime, module_trace_func, this);
}

Module::~Module() {
}

// TODO: make an actual safe method... this sucks.
// Resolve to having to ..
std::string Module::getScriptPath() {
    return _directory + "/" + _filename;
}

static bool ModuleObject(JSContext *context, uint argc, JS::Value *vp) {
    return true;
}

bool Module::run(Runtime *runtime) {
    LOG("Loading module %s/%s", _directory.c_str(), _filename.c_str());

    JSContext *context = runtime->context;

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

    // Add std classes
    if (!JS_InitStandardClasses(context, moduleScope)) {
        LOG("Could not create standard classes");
        return false;
    }

    // Add console object
    // Add __delphinus object

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
    if (!moduleObjectProto)
        FATAL("Could not create Module prototype");

    // Create a new module object
    JS::RootedObject moduleObj(context, JS_NewObjectWithGivenProto(context, &moduleClass, moduleObjectProto));
    if (!JS_DefineProperty(context, moduleScope,
                           "module", moduleObj,
                           JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT)) {
        LOG("Failed to define 'module");
        return false;
    }

    // Add 'module.id'
    JS::RootedString moduleId(context, JS_NewStringCopyZ(context, name));
    if (!JS_DefineProperty(context, moduleObj,
                           "id", moduleId,
                           JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT)) {
        LOG("Failed to define 'module.id'");
        return false;
    }

#pragma mark - Load and run script
    // Load script data from
    std::string path = getScriptPath();
    std::string scriptContents = readFile(path);

    if(scriptContents.length() == 0) {
        return false;
    }

    JS::OwningCompileOptions options(context);
    options.setFileAndLine(context, path.c_str(), 1);
    options.setIntroductionScript(nullptr); // script that initiated the require()
    options.setVersion(JSVERSION_LATEST);
    options.setUTF8(true);
    options.asmJSOption = JS::AsmJSOption::Enabled;
    options.strictOption = true;

    JS::RootedScript rtScript(context);
    if (!JS::Compile(context, options, scriptContents.c_str(),
                     strlen(scriptContents.c_str()), &rtScript)) {
        LOG("Compile error");
        return false;
    }
    script = rtScript;

//    JS_MaybeGC(context);

    JS::RootedValue rval(context);
    return JS_ExecuteScript(context, rtScript, &rval);
}

#pragma mark - Tools

std::string readFile(std::string path) {
    std::string base(SDL_GetBasePath());
    std::stringstream buffer;

    std::string fullPath = base + path;

    std::ifstream file(fullPath);

    if (!file) {
        LOG("Could not load script %s", fullPath.c_str());
        return "";
    }

    buffer << file.rdbuf();

    return buffer.str();
}

std::string stringFromValue(JSContext *context, JS::HandleValue value) {
    JS::RootedString module(context, JS::ToString(context, value));
    if (!module)
        return nullptr;

    char *moduleName = JS_EncodeStringToUTF8(context, module);
    if(!moduleName)
        return nullptr;

    std::string result(moduleName);

    JS_free(context, moduleName);
    
    return result;
}

#pragma mark - API

bool api_require(JSContext *context, uint argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    std::string module = stringFromValue(context, args.get(0));

    LOG("require() called, %u, %s", argc, module.c_str());

    args.rval().setNumber(42.0);
    return true;
}

}