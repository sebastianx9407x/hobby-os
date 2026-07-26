# Toolchain file for the x86_64-elf cross-compiler.
#
# Read by CMake BEFORE compiler detection, which is why the compiler variables
# have to live here and not in CMakeLists.txt.
#
# Usage:
#   cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/x86_64-elf.cmake

# "Generic" means: no operating system on the target. This is the line that
# stops CMake injecting host rpath / shared-library / macOS flags.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Referenced by name so PATH resolves them — works the same on macOS and Linux.
set(CMAKE_C_COMPILER   x86_64-elf-gcc)
set(CMAKE_CXX_COMPILER x86_64-elf-g++)
set(CMAKE_ASM_COMPILER x86_64-elf-gcc)

# CMake's compiler check normally compiles AND LINKS a test executable. With no
# libc and no crt0 that link always fails, so configure would die before your
# code is ever touched. Building a static library instead skips the link.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Don't let find_program/find_library wander into /opt/homebrew and hand you a
# host binary or a host libstdc++.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
