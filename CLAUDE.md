# Project: Hobby OS kernel in modern C++ (C++20/23)

## Companion docs (read these too)

@GITHUB.md
@NOTION.md

## Goal

Learn OS internals by building. Secondary: sharpen modern C++ — hand-rolled
container/utility library is a deliberate part of the project, not a chore.

**North star: run Doom on this OS.** It is the capstone — milestone 7, attempted
only after 1–6 are done. Not the shortest path to Doom, deliberately: a bare
minimum port needs only milestones 1–4 and can run as a ring-0 blob with the
WAD handed in as a Limine boot module, but doing 5 and 6 first means Doom runs
the way it should — as a real user-mode process, loaded by my ELF loader, off
my filesystem, scheduled alongside other tasks. The point is what the port
proves about the OS underneath it, so the OS gets built first.

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
- DO NOT run builds, QEMU, GDB, or the ISO pipeline. I run them. (Changed
  2026-08-08 — you used to be allowed to. Running it myself is how I learn
  to read the failures.) When you need output to diagnose something, give me
  the exact command and I'll paste the result back.
  Reading a log or artifact I already produced (build/qemu.log, a linker map,
  an objdump I pasted) is still fine — reading is not running.
- DO NOT pre-analyze my code. When I write or change something, don't go read
  it hunting for problems, don't review it unprompted, don't warn me about a
  bug you noticed. I build it; if it breaks, working out why IS the exercise,
  and you jumping in with the answer takes that away.
  Wait until I ask. Me saying "I'm stuck on X" or "why does Y happen" is the
  signal to engage — a failing build on its own is not.
- When explaining, prefer "here's the concept + here's where to read more"
  over "here's the code." Assume I want to struggle with it productively.
- RECORD WHAT YOU EXPLAIN. When I ask "what is X" / "what does Y mean" and you
  answer, append it to docs/glossary.md — short entry, plain language, plus why
  it mattered in this project. Do it as we go, not in a batch at the end, and
  don't ask permission first.
  This is a REFERENCE of your explanations, so future-me doesn't re-ask the
  same question. It is NOT my learning notes or bug write-ups — those are mine
  to write, in my words, in Notion (see NOTION.md).

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
7. DOOM (the north star, after 1–6): enable SSE/FPU + save state on context
   switch, minimal libc shims, WAD off my FS, port doomgeneric's six DG\_\* hooks,
   run it as a user-mode process via the ELF loader

## Keeping the plan in sync (do this without being asked)

Any change to the plan — roadmap items added, removed or reordered, a settled
decision reversed, scope moved between milestones — gets mirrored in the same
session it's decided:

1. Update CLAUDE.md. It's the source of truth for "what/how".
2. Update the Notion tracker to match (board layout is in NOTION.md): add,
   close, or re-parent the affected tasks. Notion is the source of truth for
   "where am I", and it must not drift from the roadmap above.
3. If the Notion STRUCTURE changed — new database, new property, new view —
   update NOTION.md as well. That file documents the layout, not the contents.

Don't wait to be asked. If a conversation changes the plan, the sync is part
of making that change, not a follow-up chore.

## Conventions

- extern "C" on all asm-visible symbols; RAII for lock/interrupt-disable guards
- No allocation in interrupt context; intrusive lists for scheduler paths
- NO C++ standard headers — not even <cstdint>. The x86_64-elf cross-compiler
  is built --without-headers and ships zero libstdc++ headers; only GCC's own C
  headers exist (stdint.h, stddef.h, stdarg.h, limits.h, stdatomic.h, cpuid.h).
  Everything else — type traits, concepts, bit ops, containers — is hand-rolled
  in lib/, SerenityOS AK/ style.
  Decided 2026-08-08, after confirming the toolchain: this is the feature, not
  the workaround. Writing the traits myself IS the C++ half of the project.
  Do not "fix" this by pointing -isystem at the host libstdc++.
- Serial is primary debug channel; read qemu.log interrupt dumps, don't guess
- Source layout — this project deliberately does NOT use the parent
  ~/Documents/Coding/CLAUDE.md's include/ + src/ convention. That split exists
  to mark a public API boundary; kernel core has no external consumers, so here
  it would be pure nesting. Sources sit directly in their component directory:
    boot/              linker script, limine.conf
    kernel/            core kernel sources (main.cpp lives here)
    kernel/arch/x86_64/ descriptor tables, port I/O, context switch asm
    lib/               mini-std
    docs/              notes
  Anything needing rewriting to port to another arch goes under arch/; policy
  and algorithms stay portable. lib/ is the one place an include/src split
  could later be justified — revisit only if it grows a real API surface.
  (GITHUB.md carries the same tree; keep the two in step.)

## Key references

- OSDev Wiki (wiki.osdev.org) — primary concept + hardware reference
- Limine boot protocol spec — authoritative for the handoff contract:
  <https://codeberg.org/Limine/limine-protocol/src/branch/trunk/PROTOCOL.md>
  NOTE: PROTOCOL.md left the limine repo in v9 and lives on Codeberg now. Older
  blog posts and the OSDev page may link the dead in-repo path. The header we
  actually compile against is limine/limine.h from the pinned binary tag.
  WARNING: that link is trunk, which runs AHEAD of the released bootloader. It
  documents base revisions 0-6; v9.6.7 implements up to 3
  (SUPPORTED_BASE_REVISION in common/protos/limine.c). Trunk describing a
  feature does NOT mean our pinned Limine has it. When it matters, check the
  source at our tag, or just test LIMINE_BASE_REVISION_SUPPORTED at boot.
- SerenityOS source (AK/ and Kernel/) — C++ patterns to learn from, not copy
- Intel SDM Vol 3 — authoritative for descriptor tables, paging, interrupts, MSRs
- Philipp Oppermann's os.phil-opp.com — clear concept explanations (ignore the Rust)
- "Writing a Simple Operating System from Scratch" (Nick Blundell PDF) — boot/early stages
