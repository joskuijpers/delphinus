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
#include <string>
#include <sstream>
#include <vector>

#include <js/Conversions.h>
#include <SDL2/SDL.h>

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

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

std::string join(const std::vector<std::string> &v, char delim) {
    std::string result;
    for (auto it = v.begin(); it != v.end(); ++it) {
        if (it == v.begin())
            result.append(*it);
        else
            result.append(delim + *it);
    }
    return result;
}
