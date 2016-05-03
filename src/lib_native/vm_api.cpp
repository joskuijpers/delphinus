//
//  vm_api.cpp
//  Delphinus
//
//  Created by Jos Kuijpers on 02/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#include "vm_api.hpp"
#include "types.hpp"
#include "util.hpp"
#include "macros.hpp"

bool api_new_vm(JSContext *context, uint argc, JS::Value *vp) {
    return true;
}

bool api_vm_get_extensions(JSContext *context, uint argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    DPH_JS_STRING(sqliteExtension, "sqlite");

    JS::AutoValueArray<1> extensions(context);
    extensions[0].setString(sqliteExtension);

    JS::RootedObject extObj(context, JS_NewArrayObject(context, extensions));
    if (!extObj) {
        return false;
    }

    args.rval().setObject(*extObj.get());
    return true;
}

bool api_vm_print(JSContext *context, uint argc, JS::Value *vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (args.length() != 1) {
        return true;
    }

    JS::RootedString jsString(context, args.get(0).toString());

    char *value = JS_EncodeStringToUTF8(context, jsString);
    if(value) {
        printf("%s", value);

        JS_free(context, value);
    }
    
    return true;
}

static const JSFunctionSpec vmFunctions[] = {
    JS_FS("get_extensions", api_vm_get_extensions, 1, 0),
    JS_FS("print", api_vm_print, 1, 0),
    JS_FS_END
};

bool delphinus::api::vm_addToScope(JSContext *context, JS::HandleObject scope) {
    static JSClass vmClass = {
        "__Delphinus", 0
    };

    // Create prototype
    JS::RootedObject prototype(context, JS_InitClass(context, scope, nullptr, &vmClass, &api_new_vm, 0, nullptr, nullptr, nullptr, nullptr));
    if (!prototype) {
        return false;
    }

    // Add methods to the prototype
    if (!JS_DefineFunctions(context, prototype, vmFunctions)) {
        return false;
    }

    // Create instance
    JS::RootedObject vm(context, JS_NewObjectWithGivenProto(context, &vmClass, prototype));
    if (!vm) {
        return false;
    }

    // Add console to the scope
    if (!JS_DefineProperty(context, scope, "__delphinus", vm, JSPROP_ENREPE)) {
        return false;
    }

    return true;
}