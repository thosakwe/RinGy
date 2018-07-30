// Copyright (c) 2018, Tobechukwu Osakwe.
//
// All rights reserved.
//
// Use of this source code is governed by an
// MIT-style license that can be found in the LICENSE file.
#include <fstream>
#include <iostream>
#include <sstream>
#include "Compiler.h"

int main(int argc, const char **argv) {
    ringy::Compiler compiler;
    std::ostringstream errorMessage;
    bool successful;

    if (argc < 2) {
        successful = compiler.Compile(std::cin, errorMessage);
    } else {
        std::ifstream ifs(argv[1]);

        if (!ifs) {
            errorMessage << "could not open file";
            successful = false;
        } else {
            successful = compiler.Compile(ifs, errorMessage);
        }
    }

    if (!successful) {
        std::cerr << "fatal error: " << errorMessage.str() << std::endl;
        return 1;
    }

    compiler.Run();
    return 0;
}