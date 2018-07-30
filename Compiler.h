#ifndef RINGY_COMPILER_H
#define RINGY_COMPILER_H

#include <jit/jit.h>
#include <istream>

namespace ringy {
  class Compiler {
    public:
      Compiler();
      ~Compiler();

      void Compile(const std::istream& stream);

    private:
      jit_context_t jitContext;
  };
}

#endif
