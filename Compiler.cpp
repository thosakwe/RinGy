// Copyright (c) 2018, Tobechukwu Osakwe.
//
// All rights reserved.
//
// Use of this source code is governed by an
// MIT-style license that can be found in the LICENSE file.
#include <jit/jit-dump.h>
#include <deque>
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
    // Create all labels in advance. This way, we can seek to  on a 'c instruction.
    std::deque<int> characters;

    while (!stream.eof() && !stream.fail()) {
        auto c = stream.get();
        characters.push_back(c);
    }

    for (auto c: characters) {
        auto *label = new jit_label_t;
        auto undefined = jit_label_undefined;
        memcpy(label, &undefined, (unsigned long) sizeof(label));
        labels.push_back(label);
    }

    unsigned long index = 0;
    jit_context_build_start(jitContext);

    // We allocate 255 bytes of memory.
    auto memorySize = jit_value_create_nint_constant(function, jit_type_int, 255);
    auto memoryBuffer = jit_insn_alloca(function, memorySize);

    // We also manage a memory pointer, a void*.
    // Initially, it points to the root of the buffer.
    auto memoryPointer = jit_value_create(function, jit_type_void_ptr);
    jit_insn_store(function, memoryPointer, memoryBuffer);

    while (!characters.empty()) {
        // Get the relevant label.
        auto *label = labels.at(index++);
        jit_insn_label(function, label);

        auto ch = characters.front();
        characters.pop_front();

        if (ch == '<') {
            // Decrease the memory pointer.
            jit_insn_add_relative(function, memoryPointer, -1);
        } else if (ch == '>') {
            // Increase the memory pointer.
            jit_insn_add_relative(function, memoryPointer, 1);
        } else if (ch == '\'') {
            // Writes the ascii value of the character 'c' to the memory element MP currently points at.
            if (characters.empty()) {
                errorMessage << "index " << index << ": no ASCII character found after '";
                return false;
            }

            // Get the char, and convert it to a char constant.
            ch = characters.front();
            characters.pop_front();
            auto jitChar = jit_value_create_nint_constant(function, jit_type_sys_char, ch);

            // Store it at the current memory pointer.
            jit_insn_store_relative(function, memoryPointer, 0, jitChar);
        } else if (ch == ':') {
            // Skips through the code until the next occurence of 'c' If the value at current memory cell isn't 0, continue reading after 'c'.
            // (Else, continue reading after the first 'c'.)
            if (characters.empty()) {
                errorMessage << "index " << index << ": no ASCII character found after :";
                return false;
            }

            // Get the char, and convert it to a char constant.
            ch = characters.front();
            characters.pop_front();

            // Loop through until the next index is found.
            unsigned long foundAt = index + 1;
            bool wasFound = false;

            std::deque<int> charsToPutBackAgain;

            while (!characters.empty()) {
                auto c = characters.front();
                characters.pop_front();
                charsToPutBackAgain.push_back(c);

                if (c == ch) {
                    wasFound = true;
                    break;
                }

                foundAt++;
            }


            if (!wasFound) {
//                errorMessage << "index " << index << ": no occurrence of the char '" << (char) ch << "' was found";
//                return false;
            } else {
                // Replace all the characters.
                while (!charsToPutBackAgain.empty()) {
                    characters.push_front(charsToPutBackAgain.front());
                    charsToPutBackAgain.pop_front();
                }

                // Now that we've found the index to jump to, maintain a reference.
                auto targetLabel = labels.at(foundAt + 1);

                // ONLY jump if the current element is NOT 0.
                auto currentElement = jit_insn_load_relative(function, memoryPointer, 0, jit_type_sys_char);
                auto zero = jit_value_create_nint_constant(function, jit_type_sys_char, 0);
                auto isZero = jit_insn_eq(function, currentElement, zero);
                jit_insn_branch_if_not(function, isZero, targetLabel);
            }
        } else if (ch == '+') {
            // Increases the value of the current memory element by 1.
            auto currentElement = jit_insn_load_relative(function, memoryPointer, 0, jit_type_sys_char);
            auto one = jit_value_create_nint_constant(function, jit_type_sys_char, 1);
            auto increased = jit_insn_add(function, currentElement, one);
            jit_insn_store_relative(function, memoryPointer, 0, increased);
        } else if (ch == '-') {
            //	Decreases the value of the current memory element by 1.
            auto currentElement = jit_insn_load_relative(function, memoryPointer, 0, jit_type_sys_char);
            auto one = jit_value_create_nint_constant(function, jit_type_sys_char, 1);
            auto decreased = jit_insn_sub(function, currentElement, one);
            jit_insn_store_relative(function, memoryPointer, 0, decreased);
        } else if (ch == '_') {
            // Inserts a memory set to 0 at the location of MP, shifting all values after it to the right by 1.
            // Insert the character 0.
            auto zeroChar = jit_value_create_nint_constant(function, jit_type_sys_char, 0);
            jit_insn_store_relative(function, memoryPointer, 0, zeroChar);

            // The end of the buffer is the original pointer, PLUS the size.
            auto end = jit_insn_add(function, memoryBuffer, memorySize);

            // The distance from the current pointer is the number of times we will need to loop.
            auto diff = jit_insn_sub(function, end, memoryPointer);

            // Maintain an index pointer, i. Initialize to 0.
            auto i = jit_value_create(function, jit_type_void_ptr);
            auto zeroPtr = jit_value_create_nint_constant(function, jit_type_void_ptr, 0);
            jit_insn_store(function, i, zeroPtr);

            // While `i` < `diff`, move the value at buf[i] to buf[i + 1].
            jit_label_t loopProc = jit_label_undefined;
            jit_insn_label(function, &loopProc);

            auto currentValue = jit_insn_load_elem(function, memoryBuffer, i, jit_type_sys_char);
            auto one = jit_value_create_nint_constant(function, jit_type_void_ptr, 1);
            auto iPlusOne = jit_insn_add(function, i, one);
            jit_insn_store_elem(function, memoryBuffer, iPlusOne, currentValue);

            // Continue looping, ONLY IF i < diff.
            auto continueLooping = jit_insn_lt(function, diff, i);
            jit_insn_branch_if(function, continueLooping, &loopProc);

            // Otherwise, increment i.
            jit_insn_store(function, i, iPlusOne);
        } else if (ch == '.') {
            // Print the character representing the value at the current memory cell.
            // Get the signature for putchar.
            auto putcharReturnType = jit_type_int;
            auto putcharSignature = jit_type_create_signature(jit_abi_cdecl, jit_type_int, &putcharReturnType, 1, 0);

            // Fetch the character.
            auto currentElement = jit_insn_load_relative(function, memoryPointer, 0, jit_type_sys_char);
            jit_insn_call_native(function, "putchar", (void *) &putchar, putcharSignature, &currentElement, 1,
                                 JIT_CALL_NOTHROW);
        } else if (ch == ',') {
            // Print the NUMERICAL character representing the value at the current memory cell.
            // Get the signature for putchar.
            auto putcharReturnType = jit_type_int;
            auto putcharSignature = jit_type_create_signature(jit_abi_cdecl, jit_type_int, &putcharReturnType, 1, 0);

            // Fetch the character.
            auto currentElement = jit_insn_load_relative(function, memoryPointer, 0, jit_type_sys_char);
            auto zeroChar = jit_value_create_nint_constant(function, jit_type_sys_char, '0');
            auto addedChars = jit_insn_add(function, currentElement, zeroChar);
            jit_insn_call_native(function, "putchar", (void *) &putchar, putcharSignature, &addedChars, 1,
                                 JIT_CALL_NOTHROW);
        } else if (ch == 'q') {
            // Quits the program. Just return.
            jit_insn_return(function, nullptr);
        } else if (ch != '\n' && ch != '\r' && ch != -1) {
            continue;
            errorMessage << "index " << index << ": illegal char '" << (char) ch << "' was found";
            return false;
        }
    }

    //jit_dump_function(stdout, function, "ringy [uncompiled]");

    if (jit_function_compile(function) == 0) {
        errorMessage << "JIT compilation failed";
        return false;
    }

    //jit_dump_function(stdout, function, "ringy [compiled]");

    return true;
}

void ringy::Compiler::Run() {
    jit_context_build_end(jitContext);
    jit_function_apply(function, nullptr, nullptr);
}
