# Project: Hobby OS kernel in modern C++ (C++20/23)

## Companion docs (read these too)

@GITHUB.md
@NOTION.md

## Goal

Learn OS internals by building. Secondary: sharpen modern C++ — hand-rolled
container/utility library is a deliberate part of the project, not a chore.

**Design constraint: aim for the lowest latency I can.** Not a feature bolted on
later — it's a lens applied to every subsystem as it's built. See "Latency as a
design constraint" below for what that actually commits me to.

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
- WHEN I DO ask you to look, aim at concepts, not mechanics. What I want:
  "you used static where you shouldn't", "this leaks implementation detail
  into the header", "that's the wrong C++ idiom for this", "this design won't
  survive milestone 3". What I don't want: stale paths, typos, a filename I
  renamed, a missing include — the build tells me those, and finding them is
  mine.
  The test isn't "will it compile" — a wrong bit mask or an inverted condition
  compiles fine and is still worth flagging. The test is whether I've
  misunderstood something, or just fat-fingered it.
- DO NOT warn me about bugs I have not hit yet. No "watch out for X", no
  "the classic mistake here is Y", no pre-emptive gotchas about the thing I am
  about to write. Hitting it and working it out IS the learning; you naming it
  in advance spends the lesson for me.
  Answer what I asked and stop there.
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
- Reference model: LINUX, for design and algorithms. (Changed 2026-08-16, was
  "reference codebase: SerenityOS".) Linux is C, so this explicitly does NOT
  mean copying its style — it means studying WHY it does what it does (buddy +
  slab allocators, RCU, per-CPU data, softirq/workqueue deferral, CFS/EEVDF
  scheduling, VFS layering, PREEMPT_RT), then implementing that design in
  modern C++.
  Where the two pull apart, C++ wins on implementation: RAII guards instead of
  paired enter/exit calls, templates and concepts instead of macros,
  Expected<T,E> instead of bare -ENOMEM returns, type-safe intrusive containers
  instead of container_of. Learn the architecture from Linux; write the code the
  way C++ should be written.
  SerenityOS is demoted to an occasional reference — useful only as an existence
  proof for C++ patterns in kernel space, not as the model to follow.
- Build the C++ library surface myself, aggressively. This is a primary goal,
  not a means to an end: every container, smart pointer, trait and utility I
  hand-roll in lib/ is the point. Prefer writing it over working around needing
  it. See milestone 4, which is deliberately broad.
- Error handling: hand-rolled Expected<T, E> modelled on C++23 std::expected,
  with monadic and_then / transform / or_else. (Changed 2026-08-16, was
  SerenityOS ErrorOr<T>/TRY().) Reasons: std::expected is the standard
  vocabulary so it transfers to hosted C++; monadic chaining needs no macro, so
  no statement expressions and CMAKE_CXX_EXTENSIONS stays OFF. <expected> is a
  library header the cross-compiler doesn't ship, so it gets written in lib/
  like everything else.
  Known tradeoff: without a `?` operator, long imperative sequences read worse
  as and_then chains than as TRY() early-returns. If that becomes painful,
  adding a TRY()-style macro on top is a deliberate re-decision, not a drift.
- Instrument as I go: rdtsc helpers early; after each subsystem works, compare
  cost AND design against Linux's version, and write down what differs and why.
  This is the main feedback loop for the "reference model: Linux" decision —
  it's how the comparison actually happens rather than staying aspirational.
- x86_64 ONLY. No portability abstraction layer (decided 2026-08-16). Portable
  code may name arch namespaces directly — kernel/panic.cpp calling
  x86_64::cpu::halt() is fine, not a leak. Rationale: the target domain is
  low-latency fintech, which is overwhelmingly x86_64 and where tuning leans
  INTO microarchitecture specifics (rdtscp/invariant TSC, cache-line size,
  prefetch, NUMA, huge pages, core isolation). An abstraction over the CPU is
  the thing that gets in the way there.
  arch/ still exists, but now as an ORGANISATIONAL boundary: "this file
  executes privileged instructions" vs "this file is pure logic". Not a
  portability seam. Don't build kernel::arch::* contract headers.
- Cross-compiler referenced by name on PATH (no hardcoded absolute paths),
  so the build works identically on macOS now and Linux later

## Roadmap

1. Boot via Limine → framebuffer text + COM1 serial + panic handler + own GDT,
   .init_array walking, runtime stubs (\_\_cxa_pure_virtual, memcpy/memset),
   rdtsc timing helpers ← current
2. IDT, exception handlers, APIC, timer + keyboard interrupts.
   Latency: APIC one-shot over periodic PIT; top/bottom-half split; measure
   IRQ-to-handler latency
3. Physical frame allocator → paging → heap; overload operator new/delete.
   Latency: bounded worst-case allocation, measured — not just good average
4. Own mini-std — deliberately broad, this is a primary goal not a support task.
   Core: Types, TypeTraits, Concepts, Expected<T,E>, Optional, Span, Array,
   Vector, String/StringView, OwnPtr/RefPtr, HashMap, IntrusiveList, Bitmap,
   RingBuffer, Function, Atomic wrappers, a type-safe format/print.
   All concepts-constrained and move-semantics-correct.
   NOTE: lib/ is not gated to this milestone — pieces get built when first
   needed (Expected in 1, Bitmap in 3, RingBuffer for buffered serial output).
   Milestone 4 is the deliberate push to fill the gaps, not the starting line.
5. Scheduler + context switch, user mode + syscalls.
   Latency: preemptible kernel, measure scheduling jitter and syscall cost
6. Later: SMP, simple FS, ELF loader
7. DOOM (the north star, after 1–6): enable SSE/FPU + save state on context
   switch, minimal libc shims, WAD off my FS, port doomgeneric's six DG\_\* hooks,
   run it as a user-mode process via the ELF loader

## Latency as a design constraint (added 2026-08-16)

**Scope: low latency, not hard real-time.** I want small and *bounded* response
times, measured. I am NOT promising provable deadlines — no WCET analysis, no
static priority ceilings, no ban on dynamic allocation. If I ever want true hard
real-time, that's a different OS and a deliberate re-scope, not a drift.

Rules, in priority order:

1. **Measure before claiming.** A subsystem isn't done until I have a number for
   it. The rdtsc helpers in "Stack & decisions" exist for this — build them in
   milestone 1, not when I finally care.
2. **Bounded worst case beats good average.** An allocator with O(1) worst case
   wins over a faster-on-average one that occasionally stalls. Latency is a
   tail-percentile problem; the mean is nearly useless.
3. **Interrupts-disabled windows are the enemy.** Every `cli` region adds
   latency to everything else in the system. Keep the RAII guards short, and
   know roughly how long each one is.
4. **Interrupt handlers stay short.** Acknowledge and defer. Top-half does the
   minimum; real work happens in a bottom-half that can be preempted.
5. **No unbounded work in interrupt context.** Already a convention — it's a
   latency rule as much as a safety one. No allocation, no unbounded loops.
6. **Prefer fine-grained timing hardware.** APIC timer one-shot / TSC-deadline
   over a periodic PIT tick — better granularity and less tick jitter.
7. **Preemptible kernel is the milestone-5 target.** It's the single biggest
   latency lever and also the hardest thing here to get right. Design toward it
   from the start rather than retrofitting.
8. **Record numbers over time** so regressions are visible. A benchmark I ran
   once and didn't write down is a benchmark I don't have.

Honest tensions — where these collide, learning wins, but the cost gets measured
and written down rather than waved away:

- Latency vs throughput: more preemption points means more overhead.
- Latency vs simplicity: a preemptible kernel is substantially harder to make
  correct, and correctness comes first.

Doom (milestone 7) doubles as the end-to-end latency workload: input-to-photon
and frame pacing are exactly the thing this constraint is about.

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
- Linux — the reference model for design. Source at elixir.bootlin.com (fast
  cross-referenced browsing). LWN.net for the "why": lwn.net/Kernel/Index/ is
  the topic index. Books: Bovet & Cesati "Understanding the Linux Kernel",
  Love "Linux Kernel Development" (gentler), Gorman "Understanding the Linux
  Virtual Memory Manager" (for milestone 3).
  Read for architecture and rationale, not style — it's C, and this is C++.
- SerenityOS source (AK/ and Kernel/) — demoted 2026-08-16. Occasional
  reference only, as an existence proof for C++ patterns in kernel space.
- Intel SDM Vol 3 — authoritative for descriptor tables, paging, interrupts, MSRs
- Philipp Oppermann's os.phil-opp.com — clear concept explanations (ignore the Rust)
- "Writing a Simple Operating System from Scratch" (Nick Blundell PDF) — boot/early stages
