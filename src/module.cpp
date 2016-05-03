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
#include "vm_api.hpp"

#include <cassert>
#include <forward_list>

#include <SDL2/SDL_filesystem.h>
#include <js/Conversions.h>

using namespace delphinus;

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

Module::Module(Runtime *runt, std::string moduleId, std::string path) {
    size_t splitPos = path.rfind("/");
    if (splitPos == std::string::npos) {
        _filename = path;
        _directory = "";
    } else {
        _filename = path.substr(splitPos + 1);
        _directory = path.substr(0, splitPos);
    }

    name = moduleId;
    runtime = runt;

    JS_AddExtraGCRootsTracer(runtime->runtime, module_trace_func, this);
}

Module::~Module() {
    JS_RemoveExtraGCRootsTracer(runtime->runtime, module_trace_func, this);
}

// TODO: make an actual safe method... this sucks.
// Resolve to having to ..
std::string Module::getScriptPath() {
    return _directory + "/" + _filename;
}

std::string Module::getModuleId() {
    return name;
}

JSObject *Module::getExports(JSContext *context) {
    JS::RootedObject rtExports(context, exports);

    return rtExports;
}

bool Module::addGlobals(JSContext *context, JS::HandleObject moduleScope) {
    // Add std classes
    if (!JS_InitStandardClasses(context, moduleScope)) {
        LOG("Could not create standard classes");
        return false;
    }

    // Add __delphinus object
    if (!api::vm_addToScope(context, moduleScope)) {
        LOG("Failed to add __delphinus to module scope");
        return false;
    }

    // These two are globals are thus loaded for every module. To prevent cyclic dependency,
    // only load it if both are not available. This does mean 'engine' can't use console.log.
    // It will have to reside with __delphinus.print.
    if (name != "engine" && name != "console") {
        // Add console object
        JS::RootedObject consoleExports(context, runtime_require(runtime, "console"));
        if (consoleExports == nullptr) {
            LOG("Failed to load console system module");
            return false;
        }

        if (!JS_DefineProperty(context, moduleScope, "console", consoleExports, JSPROP_ENREPE)) {
            LOG("Failed to set console global property");
            return false;
        }

        // Add engine object
        JS::RootedObject engineExports(context, runtime_require(runtime, "engine"));
        if (engineExports == nullptr) {
            LOG("Failed to load engine system module");
            return false;
        }

        if (!JS_DefineProperty(context, moduleScope, "engine", engineExports, JSPROP_ENREPE)) {
            LOG("Failed to set engine global property");
            return false;
        }
    }

    return true;
}

bool Module::loadIntoRuntime() {
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
    DPH_JS_STRING(__dirname, _directory.c_str())
    if(!JS_DefineProperty(context, moduleScope,
                          "__dirname", __dirname,
                          JSPROP_ENREPE)) {
        LOG("Failed to add __dirname property to module scope");
        return false;
    }

    DPH_JS_STRING(__filename, _filename.c_str());
    if (!JS_DefineProperty(context, moduleScope,
                           "__filename", __filename,
                           JSPROP_ENREPE)) {
        LOG("Failed to add __filename property to module scope");
        return false;
    }

    // Define require function
    JS::RootedObject require(context, (JSObject *)JS_DefineFunction(context,
                                                                    moduleScope,
                                                                    "require",
                                                                    &api_require, 1,
                                                                    JSPROP_ENREPE));
    if (!require) {
        LOG("Failed to define 'require'");
        return false;
    }

    if(!JS_DefineFunction(context, require,
                          "resolve", &api_resolve, 1,
                          JSPROP_ENREPE)) {
        LOG("Failed to define 'require.resolve'");
        return false;
    }

    // Add require.main to tell what the main module is.
    JS::RootedValue mainModule(context, JS::NullValue()); // TODO
    if (!JS_DefineProperty(context, require,
                           "main", mainModule,
                           JSPROP_ENREPE)) {
        LOG("Failed to define 'require.main'");
        return false;
    }

    //    JS_SetPrivate(require.get(), (void *)this);

    // Add exports and module properties
    exports = JS_NewPlainObject(context);
    JS::RootedObject rtExports(context, exports);
    if (!JS_DefineProperty(context, moduleScope,
                           "exports", rtExports,
                           JSPROP_ENREPE)) {
        LOG("Failed to define 'exports");
        return false;
    }

    // Create the Module class and prototype
    JS::RootedObject moduleObjectProto(context, JS_InitClass(context, moduleScope, nullptr, &moduleClass, ModuleObject, 0, nullptr, nullptr, nullptr, nullptr));
    if (!moduleObjectProto) {
        LOG("Could not create Module prototype");
        return false;
    }

    // Create a new module object
    JS::RootedObject moduleObj(context, JS_NewObjectWithGivenProto(context, &moduleClass, moduleObjectProto));
    if (!JS_DefineProperty(context, moduleScope,
                           "module", moduleObj,
                           JSPROP_ENREPE)) {
        LOG("Failed to define 'module");
        return false;
    }

    // Add 'module.id'
    DPH_JS_STRING(moduleId, name.c_str());
    if (!JS_DefineProperty(context, moduleObj,
                           "id", moduleId,
                           JSPROP_ENREPE)) {
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

void Module::module_trace_func(JSTracer *tracer, void *data) {
    Module *mod = (Module *)data;

    if (mod->exports) JS_CallObjectTracer(tracer, &(mod->exports), "exports");
    if (mod->scope) JS_CallObjectTracer(tracer, &(mod->scope), "scope");
    if (mod->script) JS_CallScriptTracer(tracer, &(mod->script), "script");
}

#pragma mark - Resolver

std::string resolveModuleId(std::string moduleId) {
    std::string path = std::string(SDL_GetBasePath()) + "system/" + moduleId + ".js";

    LOG("require('%s') called. Resolved to %s", moduleId.c_str(), path.c_str());

    return path;
}

#pragma mark - Module Cache

static std::unique_ptr<ModuleCache> g_moduleCache = nullptr;

ModuleCache::ModuleCache() {

}

ModuleCache::~ModuleCache() {
    list.clear();
}

std::shared_ptr<Module> ModuleCache::lookup(std::string moduleId) {
    for (auto it = list.begin(); it != list.end(); ++it) {
        if ((*it)->getModuleId().compare(moduleId) == 0) {
            return *it;
        }
    }

    return nullptr;
}

void ModuleCache::add(std::shared_ptr<Module> module) {
    list.push_front(module);
}

void delphinus::moduleCache_create() {
    if (g_moduleCache != nullptr)
        FATAL("Module cache already created");

    g_moduleCache = std::unique_ptr<ModuleCache>(new ModuleCache());
}

void delphinus::moduleCache_dispose() {
    if (g_moduleCache == nullptr)
        FATAL("Module cache not yet created");

    g_moduleCache.reset();
}

#pragma mark - API

bool api_resolve(JSContext *context, uint argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    std::string module = stringFromValue(context, args.get(0));
    std::string path = resolveModuleId(module);

    args.rval().setString(JS_NewStringCopyZ(context, path.c_str()));
    return true;
}

bool api_require(JSContext *context, uint argc, JS::Value *vp) {
    std::string moduleId;
    JS::CallArgs args;
    std::shared_ptr<Module> module;

    args = JS::CallArgsFromVp(argc, vp);
    moduleId = stringFromValue(context, args.get(0)); // TODO: normalize

    module = g_moduleCache->lookup(moduleId);
    if (module == nullptr) {
        std::string path;
        Runtime *runtime;

        runtime = Runtime::getCurrent(context);
        path = resolveModuleId(moduleId);
        module = std::shared_ptr<Module>(new Module(runtime, moduleId, path));

        if (!module->loadIntoRuntime())
            return false;

        g_moduleCache->add(module);
    }

    // Get and return the exports
    JS::RootedObject exports(context, module->getExports(context));
    if (!JS_WrapObject(context, &exports)) {
        args.rval().setUndefined();
    } else {
        args.rval().setObjectOrNull(exports);
    }

    return true;
}

JSObject *delphinus::runtime_require(Runtime *runtime, std::string moduleId) {
    std::shared_ptr<Module> module;
    JSContext *context = runtime->context;

    module = g_moduleCache->lookup(moduleId);
    if (module == nullptr) {
        std::string path;
        Runtime *runtime;

        runtime = Runtime::getCurrent(context);
        path = resolveModuleId(moduleId);
        module = std::shared_ptr<Module>(new Module(runtime, moduleId, path));

        if (!module->loadIntoRuntime())
            return nullptr;

        g_moduleCache->add(module);
    }

    // Get and return the exports
    JS::RootedObject exports(context, module->getExports(context));
    if (!JS_WrapObject(context, &exports)) {
        return nullptr;
    }

    return exports.get();
}
