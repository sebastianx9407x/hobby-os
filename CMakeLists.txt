cmake_minimum_required(VERSION 3.20)

# ASM = .S files through the GCC driver (preprocessed, GAS syntax). Swap/add
# ASM_NASM here if you decide to write NASM instead.
project(hobby_os LANGUAGES CXX ASM)

# Refuse to configure with the host compiler — the error is much clearer here
# than 200 lines into a link failure.
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Generic")
  message(
    FATAL_ERROR
      "Configure with the cross toolchain:\n"
      "  cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/x86_64-elf.cmake")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OFF gives you -std=c++23; ON gives -std=gnu++23. Flip it if you find yourself
# wanting statement expressions or typeof; __attribute__ and __asm__ work either
# way, so strict mode is the better default until something actually needs it.
set(CMAKE_CXX_EXTENSIONS OFF)

# For clangd. Symlink build/compile_commands.json to the repo root afterwards.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# You want -g and no optimisation by default; a kernel built -O2 on day one is
# miserable to step through in GDB.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE
      Debug
      CACHE STRING "Build type" FORCE)
endif()

add_subdirectory(kernel)

# TODO: add_subdirectory(lib) once the mini-std exists (milestone 4).

# ---------------------------------------------------------------------------
# Limine bootloader
# ---------------------------------------------------------------------------
# The "-binary" branch ships prebuilt bootloader blobs plus the source of the
# small host-side `limine` tool. Pinned to an exact tag on purpose: limine.conf
# syntax and the boot protocol have both changed across major versions, so a
# floating branch would silently break the boot one day.
set(LIMINE_TAG
    "v9.6.7-binary"
    CACHE STRING "Limine binary-branch tag to vendor")

include(FetchContent)
FetchContent_Declare(
  limine
  GIT_REPOSITORY https://github.com/limine-bootloader/limine.git
  GIT_TAG ${LIMINE_TAG}
  GIT_SHALLOW TRUE
  # Lives at the repo root (already in .gitignore) rather than under build/, so
  # blowing away the build directory doesn't re-clone it.
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/limine)
FetchContent_MakeAvailable(limine)

# `limine bios-install` stamps the BIOS stage-1/2 into the ISO after xorriso has
# written it. It's a HOST tool, so it must not be built with the cross compiler
# — hence a plain make invocation rather than an add_executable().
find_program(MAKE_EXECUTABLE NAMES gmake make REQUIRED)

add_custom_command(
  OUTPUT ${limine_SOURCE_DIR}/limine
  COMMAND ${MAKE_EXECUTABLE} -C ${limine_SOURCE_DIR}
  DEPENDS ${limine_SOURCE_DIR}/limine.c
  COMMENT "Building the limine host tool"
  VERBATIM)

# ---------------------------------------------------------------------------
# ISO image
# ---------------------------------------------------------------------------
find_program(XORRISO_EXECUTABLE NAMES xorriso REQUIRED)

set(ISO_ROOT ${CMAKE_BINARY_DIR}/iso_root)
set(ISO_IMAGE ${CMAKE_BINARY_DIR}/hobby-os.iso)

# Layout staged into the ISO: /boot/kernel.elf /boot/limine/limine.conf <- one
# of the paths limine searches /boot/limine/limine-bios.sys      <- BIOS stage 3
# /boot/limine/limine-bios-cd.bin   <- El Torito BIOS boot image
# /boot/limine/limine-uefi-cd.bin   <- El Torito EFI boot image
# /EFI/BOOT/BOOTX64.EFI             <- UEFI fallback path
#
# If limine.conf's `path:` and the kernel's location here ever disagree, Limine
# drops you at "no such file" — check both sides before blaming the kernel.
add_custom_command(
  OUTPUT ${ISO_IMAGE}
  COMMAND ${CMAKE_COMMAND} -E rm -rf ${ISO_ROOT}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_ROOT}/boot/limine
          ${ISO_ROOT}/EFI/BOOT
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel>
          ${ISO_ROOT}/boot/kernel.elf
  COMMAND
    ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/boot/limine.conf
    ${limine_SOURCE_DIR}/limine-bios.sys ${limine_SOURCE_DIR}/limine-bios-cd.bin
    ${limine_SOURCE_DIR}/limine-uefi-cd.bin ${ISO_ROOT}/boot/limine/
  COMMAND ${CMAKE_COMMAND} -E copy ${limine_SOURCE_DIR}/BOOTX64.EFI
          ${limine_SOURCE_DIR}/BOOTIA32.EFI ${ISO_ROOT}/EFI/BOOT/
  COMMAND
    ${XORRISO_EXECUTABLE} -as mkisofs -quiet -R -r -J -b
    boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4
    -boot-info-table --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part
    --efi-boot-image --protective-msdos-label ${ISO_ROOT} -o ${ISO_IMAGE}
  COMMAND ${limine_SOURCE_DIR}/limine bios-install ${ISO_IMAGE}
  DEPENDS kernel ${CMAKE_SOURCE_DIR}/boot/limine.conf
          ${limine_SOURCE_DIR}/limine
  COMMENT "Building hobby-os.iso"
  VERBATIM)

add_custom_target(iso ALL DEPENDS ${ISO_IMAGE})

# ---------------------------------------------------------------------------
# Run / debug targets
# ---------------------------------------------------------------------------
find_program(QEMU_EXECUTABLE NAMES qemu-system-x86_64)

if(QEMU_EXECUTABLE)
  # -d int logs every interrupt/exception to qemu.log — with -no-reboot that
  # turns a triple fault from a reboot loop into a readable trace. Note: -d int
  # only works under TCG, so don't add -accel hvf here.
  set(QEMU_FLAGS
      -M
      q35
      -m
      512M
      -cdrom
      ${ISO_IMAGE}
      -boot
      d
      -serial
      stdio
      -d
      int
      -D
      ${CMAKE_BINARY_DIR}/qemu.log
      -no-reboot
      -no-shutdown)

  add_custom_target(
    run
    COMMAND ${QEMU_EXECUTABLE} ${QEMU_FLAGS}
    DEPENDS ${ISO_IMAGE}
    COMMENT "Running hobby-os in QEMU"
    USES_TERMINAL VERBATIM)
  add_dependencies(run iso)

  # -s: gdbserver on :1234.  -S: halt at reset until GDB says continue. In
  # another pane: x86_64-elf-gdb build/kernel/kernel.elf (gdb) target remote
  # :1234
  add_custom_target(
    debug
    COMMAND ${QEMU_EXECUTABLE} ${QEMU_FLAGS} -s -S
    DEPENDS ${ISO_IMAGE}
    COMMENT "Running hobby-os in QEMU, waiting for GDB on :1234"
    USES_TERMINAL VERBATIM)
  add_dependencies(debug iso)
endif()
