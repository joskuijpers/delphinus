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

namespace delphinus {

    class Path {
        std::vector<std::string> elements;

    public:
        Path();
        Path(std::string base_dir, std::string name);
        Path(std::string path);

        ~Path();

    private:
        void constructElements(std::string path);
    };

    class Sandbox {
    public:
        Sandbox();

//        File *open(Path, mode);
//        close(File*)
//        exists(Path)
//        std::string slurp(Path)
//        byte *slurp(Path, out_size)
//        spew(Path, buf, size)
//        spew(Path, std::string)
//        mkdir(Path)
//        rmdit(Path)
//        rename(Path, Path)

    private:
//        resolve(Path) // resolve Sandboxed path to real path
    };

    class File {
        friend Sandbox;
    public:

//        putc(File*)
//        puts(File*)
//        size_t read(buf, size, count, File*) + string version
//        size_t write(buf, size, count) + string versions
//        seek(offset, whence)
//        tell
    };

}