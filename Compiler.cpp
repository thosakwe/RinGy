#include "Compiler.h"

ringy::Compiler::Compiler() {
  jitContext = jit_context_create();
}

ringy::Compiler::~Compiler() {
  jit_context_destroy(jitContext);
}

void ringy::Compiler::Compile(const std::istream& stream) {

}
