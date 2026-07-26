cmake_minimum_required(VERSION 3.20)

# ASM = .S files through the GCC driver (preprocessed, GAS syntax).
# Swap/add ASM_NASM here if you decide to write NASM instead.
project(hobby_os LANGUAGES CXX ASM)

# Refuse to configure with the host compiler — the error is much clearer here
# than 200 lines into a link failure.
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Generic")
    message(FATAL_ERROR
        "Configure with the cross toolchain:\n"
        "  cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/x86_64-elf.cmake")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# TODO: decide. OFF gives you -std=c++23; ON gives -std=gnu++23. Freestanding
# code often wants a GNU extension or two (statement exprs, __attribute__).
set(CMAKE_CXX_EXTENSIONS OFF)

# For clangd. Symlink build/compile_commands.json to the repo root afterwards.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(kernel)

# TODO: add_subdirectory(lib) once the mini-std exists (milestone 4).

# ---------------------------------------------------------------------------
# ISO image
# ---------------------------------------------------------------------------
# TODO: build the Limine ISO.
#   - You need the limine binaries (bootloader files + `limine` host tool).
#     Decide how they get here: FetchContent from the limine repo at a pinned
#     binary branch, a git submodule, or a manual download. Note .gitignore
#     already ignores limine/.
#   - Stage a directory: kernel.elf + boot/limine.conf + limine-bios.sys +
#     limine-bios-cd.bin + limine-uefi-cd.bin (+ EFI/BOOT/BOOTX64.EFI).
#   - Run xorriso over that staging dir, then `limine bios-install <iso>`.
#
# add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/hobby-os.iso
#     COMMAND xorriso ...          # TODO: flags — see the Limine template
#     DEPENDS kernel ${CMAKE_SOURCE_DIR}/boot/limine.conf
#     COMMENT "Building hobby-os.iso")
#
# add_custom_target(iso ALL DEPENDS ${CMAKE_BINARY_DIR}/hobby-os.iso)

# ---------------------------------------------------------------------------
# Run / debug targets
# ---------------------------------------------------------------------------
# TODO: wire these up so the dev loop is `cmake --build build --target run`.
# The QEMU line you settled on (see README):
#   qemu-system-x86_64 -cdrom hobby-os.iso -serial stdio \
#       -d int -D qemu.log -no-reboot -no-shutdown
# Add -s -S for a second `debug` target that waits for GDB on :1234.
#
# add_custom_target(run   DEPENDS iso COMMAND ...)
# add_custom_target(debug DEPENDS iso COMMAND ...)
