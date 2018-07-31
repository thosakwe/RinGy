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

# Check if we are on a 64 bit system
if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    message(STATUS "Building RinGy for a 64-bit system")
    target_compile_definitions(ringy PUBLIC RINGY64BIT)
else ()
    message(STATUS "Building RinGy for a 32-bit system")
endif ()

# Link the necessary libraries
target_link_libraries(ringy ${LIBJIT})

# Install it to bin/
install(TARGETS ringy RUNTIME DESTINATION bin)
