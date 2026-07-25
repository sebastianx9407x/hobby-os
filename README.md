# hobby-os

A small x86_64 operating system kernel written from scratch in freestanding
modern C++ (C++20/23), booted via [Limine](https://github.com/limine-bootloader/limine).

This is a learning project. The goal is to understand OS internals by building
them — and, along the way, to hand-roll a small container/utility library
(`Vector`, `String`, `OwnPtr`/`RefPtr`, `HashMap`) rather than lean on the
standard library.

> **Status:** early. Milestone 1 (boot + serial + panic handler) is in progress.
> Nothing builds yet.

## Design

- **Target:** x86_64, freestanding, no libc
- **Language:** C++20/23 — no exceptions, no RTTI
- **Bootloader:** Limine
- **Error handling:** `ErrorOr<T>` / `TRY()`, in the style of SerenityOS
- **Debug channel:** COM1 serial, read over QEMU's `-serial stdio`

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

Not yet wired up — see the roadmap below. The intended dev loop is:

```sh
qemu-system-x86_64 -cdrom hobby-os.iso -serial stdio \
    -d int -D qemu.log -no-reboot -no-shutdown
```

Add `-s -S` to have QEMU wait for GDB on port 1234.

## Roadmap

1. **Boot** — Limine → framebuffer text + COM1 serial + panic handler + own GDT,
   `.init_array` walking, runtime stubs (`__cxa_pure_virtual`, `memcpy`/`memset`) *(current)*
2. **Interrupts** — IDT, exception handlers, APIC, timer + keyboard IRQs
3. **Memory** — physical frame allocator → paging → heap; `operator new`/`delete`
4. **Mini-std** — `Vector`, `String`, `OwnPtr`/`RefPtr`, `HashMap`
   (concepts-constrained, move-semantics-correct)
5. **Processes** — scheduler + context switch, user mode + syscalls
6. **Later** — SMP, a simple filesystem, ELF loader

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
