# hobby-os

A small x86_64 operating system kernel written from scratch in freestanding
modern C++ (C++20/23), booted via [Limine](https://github.com/limine-bootloader/limine).

This is a learning project. The goal is to understand OS internals by building
them — and, along the way, to hand-roll a small container/utility library
(`Vector`, `String`, `OwnPtr`/`RefPtr`, `HashMap`) rather than lean on the
standard library.

The long-term target is to **run Doom on it**, as a real user-mode process
loaded off its own filesystem — not as a ring-0 blob with the WAD handed in by
the bootloader. Getting there honestly means the whole OS underneath has to
work first.

> **Status:** early. The build system boots a kernel end to end (cross-compile →
> Limine ISO → QEMU), but the kernel itself is barely started. Milestone 1
> (boot + serial + panic handler) is in progress.

## Design

- **Target:** x86_64, freestanding, no libc
- **Language:** C++20/23 — no exceptions, no RTTI
- **Bootloader:** Limine
- **Error handling:** `ErrorOr<T>` / `TRY()`, in the style of SerenityOS
- **Debug channel:** COM1 serial, read over QEMU's `-serial stdio`

**No C++ standard library headers — not even `<cstdint>`.** The `x86_64-elf`
cross-compiler is built `--without-headers` and ships none of libstdc++, so the
only headers available are GCC's own C headers (`stdint.h`, `stddef.h`,
`stdarg.h`, `limits.h`, `stdatomic.h`). Everything else — type traits, concepts,
bit manipulation, containers — is hand-written in `lib/`, in the style of
SerenityOS's `AK/`. This is the point of the exercise, not a workaround.

Compiler flags:

```
-ffreestanding -fno-exceptions -fno-rtti -nostdlib -mno-red-zone -mcmodel=kernel
```

## Prerequisites

A cross-compiler is required — the host toolchain will not produce a correct
freestanding kernel. On macOS:

```sh
brew install x86_64-elf-gcc x86_64-elf-gdb qemu xorriso nasm
```

The cross-compiler is referenced by name on `PATH` (no hardcoded absolute
paths), so the build works the same on macOS and Linux.

## Building & running

```sh
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/x86_64-elf.cmake
cmake --build build              # kernel.elf + hobby-os.iso
cmake --build build --target run # boot it in QEMU, serial on stdio
```

CMake fetches a pinned Limine release itself; there is nothing to install by
hand beyond the prerequisites above. Interrupt traces land in `build/qemu.log`.

For GDB, `--target debug` starts QEMU halted with a gdbserver on `:1234`:

```sh
cmake --build build --target debug   # one pane
x86_64-elf-gdb build/kernel/kernel.elf -ex 'target remote :1234'
```

clangd picks up `build/compile_commands.json` on its own — no symlink needed,
as long as the build directory is named `build/`.

## Roadmap

1. **Boot** — Limine → framebuffer text + COM1 serial + panic handler + own GDT,
   `.init_array` walking, runtime stubs (`__cxa_pure_virtual`, `memcpy`/`memset`) *(current)*
2. **Interrupts** — IDT, exception handlers, APIC, timer + keyboard IRQs
3. **Memory** — physical frame allocator → paging → heap; `operator new`/`delete`
4. **Mini-std** — `Vector`, `String`, `OwnPtr`/`RefPtr`, `HashMap`
   (concepts-constrained, move-semantics-correct)
5. **Processes** — scheduler + context switch, user mode + syscalls
6. **Later** — SMP, a simple filesystem, ELF loader
7. **DOOM** — enable SSE/FPU, minimal libc shims, port `doomgeneric`'s six
   `DG_*` hooks, and run it as a user-mode process off the filesystem

## Layout

```
boot/          Limine config, linker script
kernel/        core kernel sources
kernel/arch/   x86_64-specific (GDT, IDT, paging, context switch asm)
lib/           mini-std
docs/          notes, references
```

## References

- [OSDev Wiki](https://wiki.osdev.org) — primary concept + hardware reference
- [SerenityOS](https://github.com/SerenityOS/serenity) — `AK/` and `Kernel/`, for C++ patterns
- Intel SDM Vol. 3 — descriptor tables, paging, interrupts, MSRs
- [os.phil-opp.com](https://os.phil-opp.com) — clear concept explanations (the Rust is incidental)
- Nick Blundell, *Writing a Simple Operating System from Scratch* — boot/early stages

## License

MIT — see [LICENSE](LICENSE).
