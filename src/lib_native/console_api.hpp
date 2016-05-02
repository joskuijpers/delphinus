//
//  console_api.hpp
//  Delphinus
//
//  Created by Jos Kuijpers on 02/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#pragma once

#include <jsapi.h>
#include "types.hpp"

namespace delphinus {
namespace api {

class Console {
    JSClass _class;

public:
    Console();
    ~Console();

//    bool construct(JSContext *context, uint argc, JSValue *vp);
//    void finalize(JSContext *context, JSObject *object);
};

}
}