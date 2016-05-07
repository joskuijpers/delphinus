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
public:
    path_type_t type = INVALID;

public:
    Path(std::string base_dir, std::string name);
    Path(Path base_dir, std::string path);
    Path(std::string path);
    Path(std::vector<std::string> elements);

    inline bool isValid() {
        return type != INVALID;
    }

    inline bool isRoot() {
        return elements.size() == 0 && type != ABSOLUTE;
    }

    /**
     * Get the SFS path string.
     */
    std::string getString();

    inline std::vector<std::string> getElements() {
        return elements;
    }

    bool hasExtension(std::string extension);

    std::string getBasename();
    Path getDirname();

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

    /**
     * Geth whether path exists.
     */
    bool exists(Path path);

    /**
     * Geth whether path exists and is file.
     */
    bool isFile(Path path);

    /**
     * Geth whether path exists and is directory.
     */
    bool isDirectory(Path path);

//        std::string slurp(Path)
//        byte *slurp(Path, out_size)
//        spew(Path, buf, size)
//        spew(Path, std::string)
//        mkdir(Path)
//        rmdir(Path)
//        rename(Path, Path)

    class ListEntry {
        friend Sandbox;

        bool directory;
        Path path;

        ListEntry(Path _path, bool dir) : path(_path), directory(dir) {}
    public:
        inline bool isDirectory() { return directory; }
        inline bool isFile() { return !directory; }
        inline Path getPath() { return path; }
    };

    std::vector<ListEntry> list(Path path);

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