//
//  spherefs.cpp
//  Delphinus
//
//  Created by Jos Kuijpers on 02/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#include "spherefs.hpp"

delphinus::Path::Path() {
    constructElements("");
}

delphinus::Path::Path(std::string base_dir, std::string name) {
    constructElements(base_dir + "/" + name);
}

delphinus::Path::Path(std::string path) {
    constructElements(path);
}

void delphinus::Path::constructElements(std::string path) {

}

delphinus::Path::~Path() {

}
