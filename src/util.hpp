//
//  util.hpp
//  Delphinus
//
//  Created by Jos Kuijpers on 02/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#pragma once

#include <string>
#include <jsapi.h>

bool readFile(std::string path, std::string &contents);
std::string stringFromValue(JSContext *context, JS::HandleValue value);