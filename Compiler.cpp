// Copyright (c) 2018, Tobechukwu Osakwe.
//
// All rights reserved.
//
// Use of this source code is governed by an
// MIT-style license that can be found in the LICENSE file.
#include <jit/jit-dump.h>
#include "Compiler.h"

ringy::Compiler::Compiler() {
    jitContext = jit_context_create();

    // The entry point takes no parameters, and returns void.
    auto signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, nullptr, 0, 0);
    function = jit_function_create(jitContext, signature);
}

ringy::Compiler::~Compiler() {
    jit_context_destroy(jitContext);
}

bool ringy::Compiler::Compile(std::istream &stream, std::ostream &errorMessage) {
    unsigned long index = 0;

    // We allocate 255 bytes of memory.
    auto memorySize = jit_value_create_nint_constant(function, jit_type_int, 255);
    auto memoryBuffer = jit_insn_alloca(function, memorySize);

    // We also manage a memory pointer, a void*.
    // Initially, it points to the root of the buffer.
    auto memoryPointer = jit_value_create(function, jit_type_void_ptr);
    jit_insn_store(function, memoryPointer, memoryBuffer);

    while (!stream.eof() && !stream.fail()) {
        // Create a new label. This way, we can seek to it on a 'c instruction.
        auto label = jit_label_undefined;
        jit_insn_label(function, &label);
        labels.insert(std::make_pair(index, label));

        auto ch = stream.get();

        if (ch == '<') {
            // Decrease the memory pointer.
            jit_insn_add_relative(function, memoryPointer, -1);
        } else if (ch == '>') {
            // Increase the memory pointer.
            jit_insn_add_relative(function, memoryPointer, 1);
        } else if (ch == '\'') {
            // Writes the ascii value of the character 'c' to the memory element MP currently points at.
            if (stream.eof()) {
                errorMessage << "index " << index << ": no ASCII character found after '";
                return false;
            }

            // Get the char, and convert it to a char constant.
            ch = stream.get();
            auto jitChar = jit_value_create_nint_constant(function, jit_type_sys_char, ch);

            // Store it at the current memory pointer.
            jit_insn_store_relative(function, memoryPointer, 0, jitChar);
        } else if (ch == '+') {
            // 	Increases the value of the current memory element by 1.
            auto currentElement = jit_insn_load_relative(function, memoryPointer, 0, jit_type_sys_char);
            auto one = jit_value_create_nint_constant(function, jit_type_sys_char, 1);
            auto increased = jit_insn_add(function, currentElement, one);
            jit_insn_store_relative(function, memoryPointer, 0, increased);
        } else if (ch == '-') {
            // 	Decreases the value of the current memory element by 1.
            auto currentElement = jit_insn_load_relative(function, memoryPointer, 0, jit_type_sys_char);
            auto one = jit_value_create_nint_constant(function, jit_type_sys_char, 1);
            auto decreased = jit_insn_sub(function, currentElement, one);
            jit_insn_store_relative(function, memoryPointer, 0, decreased);
        }
    }

    return true;
}

void ringy::Compiler::Run() {
    jit_dump_function(stdout, function, "ringy");
    jit_function_compile(function);
    jit_function_apply(function, nullptr, nullptr);
}
