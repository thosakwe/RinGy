// Copyright (c) 2018, Tobechukwu Osakwe.
//
// All rights reserved.
//
// Use of this source code is governed by an
// MIT-style license that can be found in the LICENSE file.
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

  while (!stream.eof()) {
    // Create a new label. This way, we can seek to it on a 'c instruction.
    auto label = jit_label_undefined;
    jit_insn_label(function, &label);
    labels.insert(std::make_pair(index, label));

    auto ch = stream.get();
  }

  return true;
}

void ringy::Compiler::Run() {
  jit_function_compile(function);
  jit_function_apply(function, nullptr, nullptr);
}
