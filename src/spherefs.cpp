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
    // Split at /
    // If first element == ~sys then SYSTEM (remove 1)
    // Else if first element == ~usr/ then USER (remove 1)
    // ELse if first character in path == / THEN absolute (remove 0)
    // ELse if first element == . || ..: invalid (should have used base_dir)
    // Else GAME (remove)

    // NORMALIZE
    // Loop over elements
    // Remove any empty ones
    // if i+1 < length && i+1 == .., remove i and i+1.
}

Path::~Path() {
}

std::string Path::getString() {
    // Add prefix
    // Join elements with '/'
    return "";
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
    LOG("Create a sandbox");

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
    return "<APATH>";
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
