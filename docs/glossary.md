# Glossary / concept notes

Reference for things I asked about while building. Short definitions plus why
each one mattered here. Written by Claude as we went — my own learning notes and
bug write-ups live in Notion, not here.

Started 2026-08-08.

Companion: [port-settings.md](port-settings.md) — bit-level register layouts.
Concepts and "why" go here; lookup tables go there.

---

## Serial / UART

**UART** — Universal Asynchronous Receiver/Transmitter. The chip that does
serial: turns a byte in a register into a stream of bits on a wire, and back.
"Asynchronous" = no shared clock line between the two ends, which is why both
sides must agree on speed in advance. PCs use a 16550 (older: 8250). COM1 is at
I/O port `0x3F8`.

**Why serial is the kernel debug channel** — least machinery between you and
visible output. No driver stack, no memory management, no interrupts, no fonts.
One `out` instruction and a character appears. QEMU's `-serial stdio` pipes
whatever the guest writes to COM1 into the terminal you launched from.

**Line protocol / 8N1** — serial has no framing or clock, so both ends must be
configured identically or you get garbage rather than an error. `8N1` = 8 data
bits, No parity, 1 stop bit. A character on the wire is actually **10 bits**:
1 start + 8 data + 1 stop. So only 80% of the wire is payload, and baud/10 is
roughly bytes per second (115200 baud ≈ 11.5 KB/s). Baud counts bits including
framing.

**Start bit / stop bit** — serial has no clock line, so the receiver can't be
*told* when a byte begins; it has to detect it. The line sits at an idle level
and a **start bit** is a transition to the opposite level — that edge wakes the
receiver and starts its sampling timer. The **stop bit** is not data: it's a
fixed idle-level period guaranteeing the line returns to idle and stays there
long enough that the next start bit is unambiguously a new character rather
than more data. Two stop bits just holds idle twice as long — recovery time for
mechanical teletypes, useless now.

**IER (Interrupt Enable Register)** — UART register at offset `+1`. Each bit
enables a reason for the chip to raise an IRQ: byte received, transmit buffer
empty, line error, modem status. `0x00` = "never bother the CPU, I'll poll."
Correct for us because (a) transmitting only needs polling, and (b) we have no
IDT yet, so an IRQ would triple fault. Revisit in milestone 2 for serial input.

**Ctrl-C is not an interrupt (here)** — it's just byte `0x03` arriving like any
other. Treating it as "stop the program" is a terminal/OS software convention
far above the UART. The chip has no idea it's special.

**LCR / DLAB** — the Line Control Register (offset `+3`) holds data bits,
parity, and stop bits. Baud rate is set by a divisor written to offsets `+0`
and `+1`, which are only reachable when the DLAB bit in the LCR is set. Same
ports meaning different things depending on a bit elsewhere — the part of the
OSDev page worth reading twice.

**DB-9 / DB-25** — the physical connectors. Pedantically the 9-pin is a *DE-9*
(shell size E), but everyone says DB-9. The UART itself only produces logic
levels; a separate line driver converts them to RS-232's ±12V. None of that
exists in QEMU.

---

## Interrupts

**IRQ** — Interrupt Request. A hardware device signalling the CPU: "stop and
deal with me." Keyboard has a key, timer fired, disk finished. The CPU saves
where it was, runs a handler, resumes. The alternative is polling every device
forever. IRQ0 = timer, IRQ1 = keyboard.

**IDT** — Interrupt Descriptor Table. CPU-side array of 256 entries mapping
each interrupt/exception number to a handler address. Loaded with `lidt`.
Limine leaves it *undefined* and requires the kernel to load its own, so before
milestone 2 any exception is an instant triple fault with no diagnostics.

**IRQ vs exception** — IRQ usually means a *hardware* interrupt. An exception
(fault/trap) is the CPU interrupting itself because of what the code did —
divide by zero, page fault. Both arrive through the same IDT.

**The loop between IER and IDT** — the UART's IER decides whether an IRQ gets
raised; the CPU's IDT decides where to jump when one does.

---

## x86 mechanics

**I/O ports** — a second address space alongside memory, 65536 ports, reached
only with the `out`/`in` instructions. Not memory-mapped; there is no pointer
to dereference, which is why port access *cannot* be written in C++ and needs
inline asm. Port addresses are always `uint16_t` (the instruction takes the
port in `DX`). Data width is separate: `outb`=8, `outw`=16, `outl`=32 bits.

**Why inline asm, not for speed** — the optimizer beats hand-written assembly
for ordinary code. You drop to asm here because the instruction is
*inexpressible* in the language, not because the compiler would do it worse.
Same for `lgdt`, `lidt`, control registers, `cpuid`, `rdtsc`, `sti`/`cli`/`hlt`.

**Inline asm vs a `.S` file** — inline asm is a snippet embedded in a function
the compiler generated; it still writes the prologue, allocates the frame, and
adds the epilogue around you. A `.S` file is a standalone assembly source giving
you *every* instruction with no compiler-generated code. Capital `.S` means "run
the C preprocessor first" (so `#include`/`#define` work); lowercase `.s` skips
it. Needed for interrupt entry stubs (milestone 2) and context switch
(milestone 5), where a compiler prologue would clobber state the CPU just
pushed. GCC's assembler defaults to **AT&T syntax** — operands reversed from the
Intel SDM.

**Red zone** — 128 bytes below `RSP` that leaf functions may use without
adjusting the stack pointer. Interrupts push onto the same stack and would
corrupt it, hence `-mno-red-zone`.

---

## Toolchain / build

**No C++ standard headers** — the `x86_64-elf` cross-compiler is built
`--without-headers` and ships zero libstdc++, not even `<cstdint>`. Only GCC's
own C headers exist. Note the distinction: *language* features (`constexpr`,
`enum class`, `using`, `if constexpr`, concepts) need no headers and work fine;
anything `std::` does not.

**C++ name mangling / `extern "C"`** — the compiler encodes types into symbol
names so overloads can coexist: `kmain()` becomes `_Z5kmainv`. Linker scripts
do a literal string match and know nothing about C++, so `ENTRY(kmain)` failed
to resolve and silently defaulted to the base address — meaning whatever landed
first in `.text` ran instead. `extern "C"` gives a symbol C linkage so the name
is emitted verbatim. Does *not* change the calling convention on x86-64 SysV;
only the symbol name. Decode with `x86_64-elf-c++filt`.

**`nm` output** — case is linkage, letter is section. `T`=global text,
`t`=local text (static), `D`/`d`=initialized data, `B`/`b`=BSS, `R`/`r`=rodata,
`U`=undefined. `nm -n` sorts by address; `nm -g` shows globals only. Mangled
names starting `_ZL` are internal-linkage.

**`void*` in C vs C++** — C implicitly converts `void*` to any object pointer;
C++ does not. So Limine's `framebuffer->address` needs an explicit cast that
the wiki's C code doesn't show. `static_cast` is the right tool for `void*`→`T*`
(it can add `volatile` in the same step); `reinterpret_cast` is the bigger
hammer, for integer↔pointer or unrelated types. Reach for the weakest cast that
works.

**Designated initializers** — C++20 requires them in declaration order and
allows no skipping; C is laxer. Omitted members are value-initialized (so a
missing `.response` is already null), but `-Wextra` enables
`-Wmissing-field-initializers`, which is what flags it.

**`#define` alternatives** — `constexpr` for constants, `constexpr` functions
for function-like macros, `enum class` for related groups, `using` for type
aliases, `#pragma once` for include guards. Better because they have types, obey
scope, are visible to the debugger and clangd, and can't double-evaluate
arguments. The preprocessor is still required for `#include`, conditional
compilation, and `##`/`#`.

**Include guards** — the traditional form is `#ifndef`/`#define`/`#endif`. Name
them after the path to avoid collisions (a collision makes the second header
silently vanish). Never start a guard with underscore+capital or use a double
underscore — those are reserved for the implementation. `#pragma once` is fine
for this project: GCC and Clang both support it, and its one real failure mode
(same header reachable via two paths) doesn't apply to a plain directory tree.

**Quotes vs angle brackets** — `<...>` searches only the include path (`-I`
dirs); `"..."` searches the including file's own directory first, then falls
back. `kernel/` and `lib/` are the `-I` roots, so a header at
`kernel/arch/x86_64/io.h` is `<arch/x86_64/io.h>` — path-qualified angles, the
same style SerenityOS uses. Adding deeper `-I` roots so bare `<io.h>` works
would flatten the namespace and invite collisions.

**clangd finds `compile_commands.json` by itself** — it checks both `<dir>/` and
`<dir>/build/` while walking up from the file, so no symlink is needed as long
as the build dir is named `build/`. The `.clangd` file's `Add:` block applies to
*every* file including ones not yet in the compile database, which is what stops
a brand-new `.cpp` from falling back to host flags and resolving macOS SDK
headers.

---

## Limine / boot

**Limine vs GRUB** — same job (firmware loads the bootloader, bootloader loads
the kernel). The difference that matters: **Limine hands you 64-bit long mode
with paging on and the kernel mapped in the higher half.** GRUB+Multiboot2 drops
you in 32-bit protected mode and you write the assembly to build page tables,
enable PAE, set EFER.LME, and far-jump into 64-bit yourself. That's why
milestone 1 needs no `boot.S` and `kmain` is plain C++.

**Machine state at entry** (x86-64, from the protocol spec) — paging already on,
NX enabled, A20 open, `CS=0x28`, data segments `0x30`, interrupts off (IF
clear), legacy PICs fully masked. The IDT is explicitly undefined. Limine's GDT
lives in *bootloader-reclaimable* memory — which is the concrete reason to load
your own before reclaiming it. Kernel must load at or above
`0xffffffff80000000`; lower-half kernels are unsupported.

**Requests and `KEEP()`** — nothing in your code references the request structs;
Limine finds them by scanning the loaded image for magic numbers. So the
compiler sees a variable written but never read and the linker sees a section
nobody points at. `[[gnu::used]]`, an explicit section attribute, and `KEEP()`
in the linker script defend against that jointly — miss any one and you get a
clean build that Limine refuses to boot. Mark requests `volatile`: Limine writes
the response pointer behind the compiler's back, so without it an optimizing
build can fold your null check into "always null."

**Base revisions** — the tag declares which protocol revision you're written
against; without one you're assumed to want deprecated revision 0. Limine
negotiates *down*: if you ask for more than it supports it silently falls back
and never zeroes `p[2]`, so `LIMINE_BASE_REVISION_SUPPORTED` stays false
forever. **v9.6.7 implements up to 3** (`SUPPORTED_BASE_REVISION` in
`common/protos/limine.c`) — verified on the tag *and* the unreleased `v9.x`
branch head. `LIMINE_LOADED_BASE_REVISION` reads back what it actually settled
on, which beats cross-referencing docs.

**Two headers exist — ours has no `_ID`** — the `limine-protocol` repo (Codeberg)
carries a newer API: `LIMINE_FRAMEBUFFER_REQUEST_ID`, an initializer-style
`LIMINE_BASE_REVISION(N)` you assign, and `LIMINE_BASE_REVISION_SUPPORTED(VAR)`
taking an argument. Ours (v9.6.7 binary branch, byte-identical to the `v9.x-binary`
branch head) has `LIMINE_FRAMEBUFFER_REQUEST` with no suffix, a *declaration*-style
`LIMINE_BASE_REVISION(N)`, and an argument-less `_SUPPORTED`. **The OSDev Bare
Bones page targets the newer one.** Translation when reading the wiki: drop the
`_ID` suffix, invoke `LIMINE_BASE_REVISION` standalone rather than assigning it,
drop the argument to `_SUPPORTED`, and use revision 3 not 6. Decided to stay on
the bundled header so the bootloader and header can't drift.

---

## Project layout

**arch-specific vs portable** — the test is "would this need *rewriting* to port
to aarch64, or just recompiling?" Rewriting → `kernel/arch/x86_64/`. Descriptor
tables, port I/O, control registers, context-switch asm, page table format,
PIC/APIC are arch. Allocator *policy*, scheduler policy, VFS, and everything in
`lib/` are not. Serial splits the seam: the 16550 register layout is portable
knowledge, reaching it via `outb` at `0x3F8` is not.

**No `src/` inside `kernel/`** — the limine template needs one because its
`kernel/` is a self-contained subproject with its own build files and linker
script. Ours keeps the linker script in `boot/` and the build at the root, so
there's nothing for `src/` to separate. The `include/`+`src/` split marks a
*public API boundary*; kernel core has no external consumers. `lib/` is the only
place it could later be justified.
