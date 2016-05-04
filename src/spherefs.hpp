//
//  spherefs.hpp
//  Delphinus
//
//  Created by Jos Kuijpers on 02/05/16.
//  Copyright © 2016 Jarvix. All rights reserved.
//

#pragma once

#include <string>
#include <vector>

#include <SDL2/SDL_rwops.h>

namespace delphinus {

    class File;
    class Sandbox;

typedef enum {
    INVALID,
    SYSTEM, // ~sys/
    GAME, // *
    USER, // ~usr/
    ABSOLUTE // /
} path_type_t;

// SFS path: inside the sandbox
class Path {
    friend Sandbox;
    
    std::vector<std::string> elements;
    path_type_t type = INVALID;

public:
    Path(std::string base_dir, std::string name);
    Path(std::string path);

    inline bool isValid() {
        return type != INVALID;
    }

    /**
     * Get the SFS path string.
     */
    std::string getString();

    bool hasExtension(std::string extension);
    bool isFile();
    bool isRooted();

    // append, append_dir
    // compare (== operator)

private:
    void constructPath(std::string path);
};

// Sandbox: everything is below BasePath or PrefPath.
class Sandbox {
    friend File;

    std::string basePath;
    std::string prefPath;
public:
    Sandbox();

    /**
     * Resolve a SFS path to an OS path.
     */
    std::string resolve(Path path);

    /**
     * Whether given path is inside the sandbox.
     */
    bool containsPath(std::string nativePath);

    /**
     * Open a file in the sandbox
     */
    File *open(Path path, std::string mode);
//        exists(Path)
//        std::string slurp(Path)
//        byte *slurp(Path, out_size)
//        spew(Path, buf, size)
//        spew(Path, std::string)
//        mkdir(Path)
//        rmdir(Path)
//        rename(Path, Path)

private:
//        resolve(Path) // resolve Sandboxed path to real path
};

class File {
    friend Sandbox;

    SDL_RWops *rwOps;

    File(std::string path, std::string mode);
    ~File();
public:
    size_t read(void *buffer, size_t size, size_t num = 1);
    size_t write(const void *buffer, size_t size, size_t num = 1);
    void puts(std::string str);

    size_t seek(size_t offset, int whence);
    size_t tell();
    size_t getSize();

    void close();
};

}