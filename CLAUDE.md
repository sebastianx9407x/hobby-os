# Project: Hobby OS kernel in modern C++ (C++20/23)

## Companion docs (read these too)

@GITHUB.md
@NOTION.md

## Goal

Learn OS internals by building. Secondary: sharpen modern C++ — hand-rolled
container/utility library is a deliberate part of the project, not a chore.

## HOW I WANT YOU TO WORK WITH ME (most important)

This is a learning project. I write the code, not you.

- DO NOT write implementations for me. Do not produce full functions,
  files, or "here's the solution" code blocks unless I explicitly ask.
- Instead: explain concepts, point me to specific readings/blogs/docs
  (OSDev Wiki pages, Serenity source, Intel SDM sections, blog posts),
  and let me come back with follow-up questions.
- When I'm stuck, guide with hints, questions, and the relevant reading —
  not the answer. Help me find the bug, don't fix it for me.
- Small snippets to illustrate a concept (a few lines) are fine when I ask
  "how does X work"; full working implementations of my milestones are not.
- You CAN run builds/QEMU/GDB and read the output to help me diagnose,
  but the fix is mine to write.
- When explaining, prefer "here's the concept + here's where to read more"
  over "here's the code." Assume I want to struggle with it productively.

## Stack & decisions (settled — don't relitigate)

- Freestanding C++20/23, x86_64; cross-compiler x86_64-elf-g++ (13+)
- Flags: -ffreestanding -fno-exceptions -fno-rtti -nostdlib -mno-red-zone -mcmodel=kernel
- Bootloader: Limine (limine-cxx-template)
- Dev loop: QEMU (-serial stdio -d int -D qemu.log -no-reboot -no-shutdown), GDB via -s -S
- Reference codebase: SerenityOS (esp. AK/ library); use ErrorOr<T>/TRY() error style
- Instrument as I go: rdtsc helpers early; after each subsystem works,
  compare cost/design against Linux's version
- Cross-compiler referenced by name on PATH (no hardcoded absolute paths),
  so the build works identically on macOS now and Linux later

## Roadmap

1. Boot via Limine → framebuffer text + COM1 serial + panic handler + own GDT,
   .init_array walking, runtime stubs (\_\_cxa_pure_virtual, memcpy/memset) ← current
2. IDT, exception handlers, APIC, timer + keyboard interrupts
3. Physical frame allocator → paging → heap; overload operator new/delete
4. Own mini-std: Vector, String, OwnPtr/RefPtr, HashMap (concepts-constrained,
   move-semantics-correct)
5. Scheduler + context switch, user mode + syscalls
6. Later: SMP, simple FS, ELF loader

## Conventions

- extern "C" on all asm-visible symbols; RAII for lock/interrupt-disable guards
- No allocation in interrupt context; intrusive lists for scheduler paths
- Freestanding std headers OK (<type_traits>, <bit>, <atomic>, <concepts>);
  anything allocating is ours
- Serial is primary debug channel; read qemu.log interrupt dumps, don't guess

## Key references

- OSDev Wiki (wiki.osdev.org) — primary concept + hardware reference
- SerenityOS source (AK/ and Kernel/) — C++ patterns to learn from, not copy
- Intel SDM Vol 3 — authoritative for descriptor tables, paging, interrupts, MSRs
- Philipp Oppermann's os.phil-opp.com — clear concept explanations (ignore the Rust)
- "Writing a Simple Operating System from Scratch" (Nick Blundell PDF) — boot/early stages
