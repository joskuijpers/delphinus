//
//  util.cpp
//  Delphinus
//
//  Created by Jos Kuijpers on 02/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#include "util.hpp"
#include "macros.hpp"

#include <fstream>
#include <sstream>

#include <js/Conversions.h>
#include <SDL2/SDL.h>

bool readFile(std::string path, std::string &contents) {
    std::stringstream buffer;
    std::ifstream file(path);

    if (!file) {
        LOG("Could not load script %s", path.c_str());
        return false;
    }

    buffer << file.rdbuf();
    contents = buffer.str();

    return true;
}

std::string stringFromValue(JSContext *context, JS::HandleValue value) {
    if (!value.isString())
        return nullptr;

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