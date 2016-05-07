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
#include "spherefs.hpp"

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
    "Module", JSCLASS_HAS_PRIVATE
};

static bool ModuleObject(JSContext *context, uint argc, JS::Value *vp) {
    return true;
}

#pragma mark - Module Class

Module::Module(Runtime *runt, std::string moduleId) {
    name = moduleId;
    runtime = runt;

    JS_AddExtraGCRootsTracer(runtime->runtime, module_trace_func, this);
}

Module::~Module() {
    JS_RemoveExtraGCRootsTracer(runtime->runtime, module_trace_func, this);
}

std::string Module::getScriptPath() {
    return runtime->sandbox->resolve(Path(name));
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
    if (name != "~sys/engine.js" && name != "~sys/console.js") {
        // Add console object
        JS::RootedObject consoleExports(context, runtime_require(runtime, "~sys/console.js"));
        if (consoleExports == nullptr) {
            LOG("Failed to load console system module");
            return false;
        }

        if (!JS_DefineProperty(context, moduleScope, "console", consoleExports, JSPROP_ENREPE)) {
            LOG("Failed to set console global property");
            return false;
        }

        // Add engine object
        JS::RootedObject engineExports(context, runtime_require(runtime, "~sys/engine.js"));
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
    DPH_JS_STRING(__dirname, Path(name).getDirname().getString().c_str())
    if(!JS_DefineProperty(context, moduleScope,
                          "__dirname", __dirname,
                          JSPROP_ENREPE)) {
        LOG("Failed to add __dirname property to module scope");
        return false;
    }

    DPH_JS_STRING(__filename, Path(name).getBasename().c_str());
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

    // Add require.resolve
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

    // Add module reference to this
    JS_SetPrivate(moduleObj.get(), (void *)this);

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

bool module_isCoreModule(std::string moduleId) {
    if (   moduleId == "assert"
        || moduleId == "util"
        || moduleId == "random") {
        return true;
    }

    return false;
}

bool module_resolveAsFile(Runtime *runtime, Path path, std::string &moduleId) {
    if (!path.isValid()) {
        return false;
    }

    // Try without .js added
    if (runtime->getSandbox()->isFile(path)) {
        moduleId = path.getString();
        return true;
    }

    // Try with .js added
    Path newPath = Path(path.getString() + ".js");
    if (newPath.isValid() && runtime->getSandbox()->isFile(newPath)) {
        moduleId = newPath.getString();
        return true;
    }

    return false;
}

bool module_resolveAsDirectory(Runtime *runtime, Path path, std::string &moduleId) {
    if (!path.isValid()) {
        return false;
    }

    // If X/package.json is a file, load main field.
    Path packageJson(path, "package.json");
    if (runtime->getSandbox()->isFile(packageJson)) {
        LOG("parse %s", packageJson.getString().c_str());

        std::string mainField = "main.js";
        Path mainPath(path, mainField);

        return module_resolveAsFile(runtime, mainPath, moduleId);
    }

    // If X/index.js is a file, load
    Path indexFile(path, "index.js");
    if (runtime->getSandbox()->isFile(indexFile)) {
        moduleId = indexFile.getString();
        return true;
    }

    return false;
}

std::vector<Path> module_getPackagePaths(Runtime *runtime, Path basePath) {
    std::vector<Path> paths;
    std::vector<std::string> elements = basePath.getElements();

    for (long i = elements.size() - 1; i >= 0; --i) {
        // If we were in a packages folder, don't scan it.
        if (elements[i] == "packages")
            continue;

        // Slice from 0-i.
        std::vector<std::string> slice;
        for (long j = 0; j <= i; ++j) {
            slice.push_back(elements[j]);
        }
        slice.push_back("packages"); // Packages folder

        // Create path and add it
        Path dir(slice);
        if (dir.isValid()) {
            paths.push_back(dir);
        }
    }

    // Also add root package module
    paths.push_back(Path("packages"));

    return paths;
}

bool module_resolveAsPackage(Runtime *runtime, std::string relModuleId, Path basePath, std::string &moduleId) {
    if (!basePath.isValid()) {
        return false;
    }

    // Get directories at nodeModPaths(basePath)
    std::vector<Path> dirs = module_getPackagePaths(runtime, basePath);
    for (auto it = dirs.begin(); it < dirs.end(); ++it) {
        Path p(*it, relModuleId);

        if (module_resolveAsFile(runtime, p, moduleId)) {
            return true;
        }

        if (module_resolveAsDirectory(runtime, p, moduleId)) {
            return true;
        }
    }

    return false;
}

bool module_resolveModuleId(Runtime *runtime, std::string relModuleId, Module *fromModule, std::string &moduleId) {
    Path fromPath(fromModule->getModuleId());
    Path fromDir = fromPath.getDirname();

    // Lookup in cache the relModuleId + fromModule tuple

    // If the module is a core module, resolve it.
    if (module_isCoreModule(relModuleId)) {
        moduleId = "~sys/" + relModuleId + ".js";
        return true;
    }

    // If it is a relative module, try to find it
    if (relModuleId.find("./") == 0 || relModuleId.find("../") == 0) {
        if (module_resolveAsFile(runtime, Path(fromDir, relModuleId), moduleId)) {
            return true;
        }

        if (module_resolveAsDirectory(runtime, Path(fromDir, relModuleId), moduleId)) {
            return true;
        }
    }

    // Try the packages folder
    else if (module_resolveAsPackage(runtime, relModuleId, fromDir, moduleId)) {
        return true;
    }

    JS_ReportError(runtime->context, "Cannot find module '%s'", relModuleId.c_str());

    return false;
}

#pragma mark - Module Cache

static std::unique_ptr<ModuleCache> g_moduleCache = nullptr;

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

/**
 * Get the module of the current global scope.
 *
 * @return Module pointer. Owned by the system. Do not free it, ever.
 */
Module *get_current_module(JSContext *context, JS::HandleObject global) {
    // Get module object
    JS::RootedValue moduleValue(context);
    if (!JS_GetProperty(context, global, "module", &moduleValue)) {
        return nullptr;
    }

    // Get private
    if (!moduleValue.isObject()) {
        return nullptr;
    }

    JS::RootedObject moduleObject(context);
    if (!JS_ValueToObject(context, moduleValue, &moduleObject)) {
        return nullptr;
    }

    return (Module *)JS_GetPrivate(moduleObject);
}

bool api_resolve(JSContext *context, uint argc, JS::Value *vp) {
    std::string relModuleId;
    Runtime *runtime;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    // Find the global and acquire the current Module
    JS::RootedObject global(context, JS_GetGlobalForObject(context, &args.callee()));
    Module *fromModule = get_current_module(context, global);

    relModuleId = stringFromValue(context, args.get(0));
    runtime = Runtime::getCurrent(context);

    std::string moduleId;
    if (!module_resolveModuleId(runtime, relModuleId, fromModule, moduleId))
        return false;

    args.rval().setString(JS_NewStringCopyZ(context, moduleId.c_str()));
    return true;
}

bool api_require(JSContext *context, uint argc, JS::Value *vp) {
    std::string relModuleId, moduleId;
    JS::CallArgs args;
    std::shared_ptr<Module> module;
    Runtime *runtime;

    args = JS::CallArgsFromVp(argc, vp);

    // Get the moduleId reqeuested
    relModuleId = stringFromValue(context, args.get(0));

    // Find the global and acquire the current Module
    JS::RootedObject global(context, JS_GetGlobalForObject(context, &args.callee()));
    Module *fromModule = get_current_module(context, global);

    runtime = Runtime::getCurrent(context);

    // Resolve it to a normalized path (which is also the moduleId)
    if (!module_resolveModuleId(runtime, relModuleId, fromModule, moduleId)) {
        return false;
    }

    module = g_moduleCache->lookup(moduleId);
    if (module == nullptr) {
        module = std::shared_ptr<Module>(new Module(runtime, moduleId));

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

    // This moduleId is always normalized
    module = g_moduleCache->lookup(moduleId);
    if (module == nullptr) {
        Runtime *runtime;

        runtime = Runtime::getCurrent(context);
        module = std::shared_ptr<Module>(new Module(runtime, moduleId));

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
