// Copyright (c) 2018, Tobechukwu Osakwe.
//
// All rights reserved.
//
// Use of this source code is governed by an
// MIT-style license that can be found in the LICENSE file.
#ifndef RINGY_COMPILER_H
#define RINGY_COMPILER_H

#include <jit/jit.h>
#include <istream>
#include <vector>

namespace ringy
{
    class Compiler
    {
    public:
        Compiler();

        ~Compiler();

        bool Compile(std::istream &stream, std::ostream &errorMessage);

        void Run();

    private:
        std::vector<jit_label_t*> labels;
        jit_context_t jitContext;
        jit_function_t function;
    };
}

#endif
