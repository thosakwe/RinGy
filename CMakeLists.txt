cmake_minimum_required(VERSION 3.0)
project(RinGy)
set(CMAKE_CXX_STANDARD 11)

# Find the LibJIT headers
find_path(LIBJIT_INCLUDE_DIR jit/jit.h)
include_directories(${LIBJIT_INCLUDE_DIR})

# And the LibJIT library
find_library(LIBJIT jit)

# Create our executable
add_executable(ringy
  main.cpp
  Compiler.cpp Compiler.h
)

# Link the necessary libraries
target_link_libraries(ringy ${LIBJIT})

# Install it to bin/
install(TARGETS ringy RUNTIME DESTINATION bin)
