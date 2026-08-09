add_executable(kernel)

set_target_properties(kernel PROPERTIES OUTPUT_NAME kernel.elf)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------
# TODO: add files as you write them. Listed explicitly on purpose — GLOB doesn't
# re-run when you add a file.
target_sources(
  kernel
  PRIVATE main.cpp
          # arch/x86_64/gdt.cpp arch/x86_64/serial.cpp arch/x86_64/panic.cpp
          # arch/x86_64/boot.S
)

target_include_directories(
  kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/lib
                 ${CMAKE_SOURCE_DIR}/limine)

# ---------------------------------------------------------------------------
# Compile options
# ---------------------------------------------------------------------------
# COMPILE flags only — link flags go below, and mixing the two up is the classic
# way to waste an evening.
target_compile_options(
  kernel
  PRIVATE -ffreestanding # no hosted libc, no assumptions about main()
          -fno-exceptions # unwinding needs a runtime you don't have
          -fno-rtti # typeinfo needs a runtime you don't have
          -fno-stack-protector # otherwise you must provide __stack_chk_fail
          -fno-stack-check
          -fno-PIC
          -fno-PIE # fixed load address; no GOT/PLT indirection
          # The kernel lives in the top 2 GB, so RIP-relative addressing with
          # 32-bit signed displacements works. This MUST agree with the base
          # address in boot/linker.ld or you get relocation-truncated-to-fit at
          # link time.
          -mcmodel=kernel
          # The red zone is the 128 bytes below RSP that leaf functions may use
          # without adjusting RSP. Interrupts push onto the stack and would
          # corrupt it, so it has to go before you ever take an interrupt.
          -mno-red-zone
          # No FPU/SSE state is saved or even enabled yet, so the compiler must
          # not spill through an XMM register behind your back.
          -mno-80387
          -mno-mmx
          -mno-sse
          -mno-sse2
          -fno-omit-frame-pointer # makes hand-rolled stack traces possible
          -Wall
          -Wextra)

# ---------------------------------------------------------------------------
# Link options
# ---------------------------------------------------------------------------
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/boot/linker.ld)

target_link_options(
  kernel
  PRIVATE
  -nostdlib # no crt0, no libc, no libstdc++
  -static
  -no-pie
  -T
  ${LINKER_SCRIPT}
  # Limine loads segments at page granularity; the default 2 MB max page size
  # would pad the image absurdly.
  -Wl,-z,max-page-size=0x1000
  # Keeps a .note.gnu.build-id out of the image, which your linker script would
  # otherwise have to place or discard explicitly.
  -Wl,--build-id=none)

# libgcc supplies the compiler's own helper routines (128-bit divide, etc.) that
# GCC emits calls to. It is freestanding-safe; -nostdlib drops it, so ask for it
# back explicitly. Must come after the objects on the link line.
target_link_libraries(kernel PRIVATE gcc)

# -T alone does NOT make CMake relink when the linker script changes. Without
# this you will eventually debug a stale binary and lose an hour to it.
set_target_properties(kernel PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

# A clearer failure than CMake's generic "no SOURCES given to target".
get_target_property(_kernel_sources kernel SOURCES)
if(NOT _kernel_sources)
  message(
    FATAL_ERROR
      "The kernel target has no sources yet.\n"
      "Write your first file (e.g. kernel/main.cpp) and uncomment it in the\n"
      "target_sources() list in kernel/CMakeLists.txt.")
endif()
