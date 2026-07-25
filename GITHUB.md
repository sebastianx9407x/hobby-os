# GitHub / repo conventions

## Attribution (important)

- Do NOT add any Claude/AI attribution anywhere: no "Generated with Claude",
  no "Co-Authored-By: Claude", no AI credit in commits, PR descriptions,
  READMEs, comments, or anywhere else.
- Commits are authored as me, in my voice. Nothing indicates AI involvement.

## Repo basics

- This is my personal hobby OS kernel (C++20/23, x86_64, Limine).
- License: my choice (MIT is a fine default for a hobby project) — ask me
  before adding one, don't assume.

## Commits

- I write the code, so I make the commits. If you help draft a commit message,
  keep it concise and factual, present-tense imperative
  ("Add IDT setup", "Fix triple fault on GDT reload"), no AI attribution.
- Small, focused commits over giant ones — one logical change each.
- Reference the milestone where useful ("Milestone 2: wire up timer IRQ").

## .gitignore (build artifacts — don't commit these)

- Build output: `*.o`, `*.elf`, `*.iso`, `*.bin`, `build/`, etc.
- QEMU logs: qemu.log
- Editor/OS cruft: nvim swap/undo files (`*.swp`, `*.un~`), `Session.vim`,
  project-local `.nvim.lua` / `.lazy.lua`, `.DS_Store`
- Toolchain: any locally built cross-compiler dir

## Suggested structure (my call, this is just a starting point)

- boot/ Limine config, linker script
- kernel/ core kernel sources
- kernel/arch/ x86_64-specific (GDT, IDT, paging, context switch asm)
- lib/ my mini-std (Vector, String, OwnPtr, HashMap, ...)
- docs/ notes, references
- CLAUDE.md, NOTION.md, GITHUB.md at root

## What NOT to commit

- Secrets/tokens of any kind
- Large binaries or the cross-compiler toolchain itself
- The .iso / build artifacts (see .gitignore)
