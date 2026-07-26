add_executable(kernel)

set_target_properties(kernel PROPERTIES OUTPUT_NAME kernel.elf)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------
# TODO: add files as you write them. Listed explicitly on purpose — GLOB
# doesn't re-run when you add a file.
target_sources(kernel PRIVATE
    # main.cpp
    # arch/x86_64/gdt.cpp
    # arch/x86_64/serial.cpp
    # arch/x86_64/panic.cpp
    # arch/x86_64/boot.S
)

target_include_directories(kernel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
)

# ---------------------------------------------------------------------------
# Compile options
# ---------------------------------------------------------------------------
# TODO: fill in. These are COMPILE flags only — link flags go below, and mixing
# the two up is the classic way to waste an evening.
#
# From your README, settled: -ffreestanding -fno-exceptions -fno-rtti
#                            -mno-red-zone -mcmodel=kernel
#
# Also think about (and decide for yourself why each one matters):
#   -fno-stack-protector      no __stack_chk_fail to link against
#   -fno-stack-check
#   -fno-PIC / -fno-pie       you're loading at a fixed address
#   -mno-80387 -mno-mmx       no FPU/SIMD state is set up yet, so the compiler
#   -mno-sse -mno-sse2        must not emit an SSE spill behind your back
#   -Wall -Wextra
#
# target_compile_options(kernel PRIVATE ...)

# ---------------------------------------------------------------------------
# Link options
# ---------------------------------------------------------------------------
# TODO: fill in. Starting points: -nostdlib -static -no-pie -z max-page-size=0x1000
# and -T ${CMAKE_SOURCE_DIR}/boot/linker.ld
#
# target_link_options(kernel PRIVATE ...)

# -T alone does NOT make CMake relink when the linker script changes. Without
# this you will eventually debug a stale binary and lose an hour to it.
set_target_properties(kernel PROPERTIES
    LINK_DEPENDS ${CMAKE_SOURCE_DIR}/boot/linker.ld)
