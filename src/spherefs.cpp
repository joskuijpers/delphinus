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
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_filesystem.h>

using namespace delphinus;

#pragma mark - Path

Path::Path() {
    type = INVALID;
}

Path::Path(std::string base_dir, std::string name) {
    std::string path = base_dir;
    if (path.length() == 0) {
        path = name;
    } else {
        path += "/" + name;
    }

    constructPath(path);
}

Path::Path(std::string path) {
    constructPath(path);
}

Path::Path(Path base_dir, std::string name) {
    std::string path = base_dir.getString();
    if (path.length() == 0) {
        path = name;
    } else {
        path += "/" + name;
    }

    constructPath(path);
}

Path::Path(std::vector<std::string> elements) {
    // Could just set elements but that is unsafe
    constructPath(join(elements, '/'));
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
            error = "Path '" + path + "' is invalid: found prefixes mid-text";
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
            error = "Path '" + path + "' is invalid: path resolved outside sandbox";
            type = INVALID;
            return;
        }

        // . operators are skipped. If at start of path, assume root is ""
        if (elems[i] == ".") {
            continue;
        }

        if (elem.find("\\") != std::string::npos) {
            error = "Path '" + path + "' is invalid: \\ characters are not supported";
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

std::string Path::getBasename() {
    assert(isValid());

    if (elements.size() > 0)
        return elements[elements.size() - 1];
    return "";
}

Path Path::getDirname() {
    std::string basename = getBasename();
    std::string path = getString();
    path = path.substr(0, path.length() - basename.length());

    return Path(path);
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
    if (nativePath.find(basePath) == 0 || nativePath.find(prefPath) == 0) {
        return true;
    }
    return false;
}

std::string Sandbox::resolve(Path path) {
    std::string subPath, rootPath;

    assert(path.isValid());

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
    std::string resPath;
    assert(path.isValid());

    resPath = resolve(path);
    if (resPath.length() < 1) {
        return nullptr;
    }

    return new File(resPath, mode);
}

bool Sandbox::exists(Path path) {
    struct stat buffer;
    std::string osPath = resolve(path);

    return stat(osPath.c_str(), &buffer) == 0;
}

bool Sandbox::isFile(Path path) {
    struct stat buffer;
    std::string osPath = resolve(path);

    if (stat(osPath.c_str(), &buffer) < 0) {
        return false;
    }

    return S_ISREG(buffer.st_mode);
}

bool Sandbox::isDirectory(Path path) {
    struct stat buffer;
    std::string osPath = resolve(path);

    if (stat(osPath.c_str(), &buffer) < 0) {
        return false;
    }

    return S_ISDIR(buffer.st_mode);
}

std::vector<Sandbox::ListEntry> Sandbox::list(Path path) {
    std::string osPath = resolve(path);
    std::vector<Sandbox::ListEntry> entries;

    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(osPath.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            std::string name(ent->d_name);
            if (ent->d_type == DT_DIR) {
                entries.push_back(Sandbox::ListEntry(Path(path, name + "/"), true));
            } else if (ent->d_type == DT_REG) {
                entries.push_back(Sandbox::ListEntry(Path(path, name), false));
            }
        }
        closedir(dir);
    }

    return entries;
}

bool Sandbox::slurp(Path path, std::string& result) {
    assert(path.isValid());

    // Get file size
    File *f = open(path, "r");
    if (f == nullptr) {
        return false;
    }

    size_t fileSize = f->getSize();
    f->close();

    // Create a buffer
    byte *buffer = new byte[fileSize];
    if (buffer == nullptr) {
        return false;
    }

    // Load the data
    if (slurp(path, buffer, fileSize) != fileSize) {
        delete[] buffer;
        return false;
    }

    // Convert to string
    result = std::string(reinterpret_cast<const char *>(buffer), fileSize);

    delete[] buffer;

    return true;
}

size_t Sandbox::slurp(Path path, byte *buffer, size_t size) {
    if (!path.isValid()) {
        return 0;
    }

    File *file = open(path, "rb");
    if (file == nullptr) {
        return 0;
    }

    size_t bytesRead = 0;
    size_t readNow = 1;

    while (bytesRead < size && readNow != 0) {
        readNow = file->read(buffer + bytesRead, 1, size - bytesRead);
        bytesRead += readNow;
    }

    file->close();

    // Not finished the file
    if (bytesRead != size) {
        return 0;
    }

    return bytesRead;
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

    SDL_RWclose((SDL_RWops *)rwOps);
    rwOps = nullptr;

    delete this;
}

size_t File::seek(size_t offset, int whence) {
    assert(rwOps != nullptr);

    return SDL_RWseek((SDL_RWops *)rwOps, offset, whence);
}

size_t File::getSize() {
    assert(rwOps != nullptr);

    return SDL_RWsize((SDL_RWops *)rwOps);
}

size_t File::tell() {
    assert(rwOps != nullptr);

    return SDL_RWtell((SDL_RWops *)rwOps);
}

size_t File::read(void *buffer, size_t size, size_t num) {
    assert(rwOps != nullptr && buffer != nullptr);

    if (size == 0)
        return 0;

    return SDL_RWread((SDL_RWops *)rwOps, buffer, size, num);
}

size_t File::write(const void *buffer, size_t size, size_t num) {
    assert(rwOps != nullptr && buffer != nullptr);

    if (size == 0)
        return 0;

    return SDL_RWwrite((SDL_RWops *)rwOps, buffer, size, num);
}

void File::puts(std::string str) {
    assert(rwOps != nullptr);

    const char *cStr = str.c_str();
    write(cStr, strlen(cStr), 1);
}
