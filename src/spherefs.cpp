//
//  spherefs.cpp
//  Delphinus
//
//  Created by Jos Kuijpers on 02/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#include "spherefs.hpp"
#include "types.hpp"
#include "macros.hpp"
#include "util.hpp"

#include <cassert>

#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_filesystem.h>

using namespace delphinus;

#pragma mark - Path

Path::Path(std::string base_dir, std::string name) {
    constructPath(base_dir + "/" + name);
}

Path::Path(std::string path) {
    constructPath(path);
}

void Path::constructPath(std::string path) {
    size_t i = 0;
    std::vector<std::string> elems = split(path, '/');

    // Determine the type using the prefix
    if (path.find("/") == 0) {
        type = ABSOLUTE;
        i += 1;
    } else if (path.find("~usr/") == 0) {
        type = USER;
        i += 1;
    } else if (path.find("~sys/") == 0) {
        type = SYSTEM;
        i += 1;
    } else {
        type = GAME;
    }

    // Normalize and find path properties. This is the only place
    // where we can 'remove' elements (by not adding them to the final
    // vector).
    for(; i < elems.size(); ++i) {
        std::string elem = elems[i];

        // Remove empty elements
        if (elem.length() == 0) {
            continue;
        }

        // Find invalid path components
        if (elem.length() == 4 && (elem.compare("~sys") == 0 || elem.compare("~usr") == 0)) {
            type = INVALID;
            return;
        }

        // If next element is .., skip this and next element.
        if (i + 1 < elems.size() && elems[i+1] == "..") {
            i += 1;
            continue;
        }

        // All .. should have been removed by above statement
        if (elems[i] == "..") {
            type = INVALID;
            return;
        }

        // . operators are skipped, but if at start path is invalid
        if (elems[i] == ".") {
            if (i == 0) {
                type = INVALID;
                return;
            }
            continue;
        }

        if (elem.find("\\") != std::string::npos) {
            type = INVALID;
            return;
        }

        elements.push_back(elem);
    }
}

std::string Path::getString() {
    std::string prefix, path;

    switch (type) {
        case INVALID:
            return nullptr;
        case SYSTEM:
            prefix = "~sys/";
            break;
        case GAME:
            prefix = "";
            break;
        case USER:
            prefix = "~usr/";
            break;
        case ABSOLUTE:
            prefix = "/";
            break;
    }

    path = join(elements, '/');

    return prefix + path;
}

bool Path::hasExtension(std::string extension) {
    return false;
}

bool Path::isFile() {
    return false;
}

bool Path::isRooted() {
    return false;
}

#pragma mark - Sandbox

Sandbox::Sandbox() {
    const char *path = SDL_GetBasePath();
    basePath = std::string(path);
    SDL_free((void *)path);

    path = SDL_GetPrefPath("Jarvix", "MyGame"); // Todo: use some actual values
    prefPath = std::string(path);
    SDL_free((void *)path);
}

bool Sandbox::containsPath(std::string nativePath) {
    return false;
}

std::string Sandbox::resolve(Path path) {
    std::string subPath, rootPath;

    if (!path.isValid())
        return nullptr;

    subPath = join(path.elements, '/');

    if (path.type == SYSTEM) {
        rootPath = basePath + "system/";
    } else if (path.type == GAME) {
        rootPath = basePath + "game/";
    } else if (path.type == USER) {
        rootPath = prefPath;
    } else if (path.type == ABSOLUTE) {
        rootPath = "/";
    }

    return rootPath + subPath;
}

File *Sandbox::open(Path path, std::string mode) {
    std::string resPath = resolve(path);
    if (resPath.length() < 1) {
        return nullptr;
    }

    return new File(resPath, mode);
}

#pragma mark - File

File::File(std::string path, std::string mode) {
    rwOps = SDL_RWFromFile(path.c_str(), mode.c_str());
}

File::~File() {
    if (rwOps != nullptr) {
        LOG("File not closed! Leaking!");
        close();
    }
}

void File::close() {
    assert(rwOps != nullptr);

    SDL_RWclose(rwOps);
    rwOps = nullptr;
}

size_t File::seek(size_t offset, int whence) {
    assert(rwOps != nullptr);

    return SDL_RWseek(rwOps, offset, whence);
}

size_t File::getSize() {
    assert(rwOps != nullptr);

    return SDL_RWsize(rwOps);
}

size_t File::tell() {
    assert(rwOps != nullptr);

    return SDL_RWtell(rwOps);
}

size_t File::read(void *buffer, size_t size, size_t num) {
    assert(rwOps != nullptr && buffer != nullptr);

    if (size == 0)
        return 0;

    return SDL_RWread(rwOps, buffer, size, num);
}

size_t File::write(const void *buffer, size_t size, size_t num) {
    assert(rwOps != nullptr && buffer != nullptr);

    if (size == 0)
        return 0;

    return SDL_RWwrite(rwOps, buffer, size, num);
}

void File::puts(std::string str) {
    assert(rwOps != nullptr);

    const char *cStr = str.c_str();
    write(cStr, strlen(cStr), 1);
}
