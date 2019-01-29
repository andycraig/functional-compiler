# The compiler
project(compile)
file(GLOB_RECURSE sources src/*.c src/*.h)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
add_executable(compile ${sources})

# Static libraries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# Static library that provides closure functionality to Assembly files
project(closure)
add_library(closure STATIC src/closure.c)

# Static library of basic functions, written in NASM
project(standard C)
enable_language(ASM_NASM)
if(CMAKE_ASM_NASM_COMPILER_LOADED)
  add_library(standard STATIC src/standard.asm)
endif(CMAKE_ASM_NASM_COMPILER_LOADED)