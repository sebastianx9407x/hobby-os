/* Linker script for the hobby-os kernel (Limine, x86_64).
 *
 * This file is yours to work out — it's core milestone-1 material, not
 * plumbing. Skeleton and decision points only.
 *
 * Read first:
 *   https://wiki.osdev.org/Linker_Scripts
 *   https://wiki.osdev.org/Limine_Bare_Bones   (its linker.ld is the model)
 */

/* Tell the linker that we want an x86_64 ELF64 output file */
OUTPUT_FORMAT(elf64-x86-64)

/* We want the symbol kmain to be our entry point */
ENTRY(kmain)

PHDRS
{
    /* TODO: declare segments. Limine wants separate loadable segments so it
     * can map them with the right permissions (text RX, rodata R, data RW).
     * Each needs PT_LOAD and a FLAGS(...) bitmask: 1=X, 2=W, 4=R.
     *
     * You may also need a PT_NOTE-style entry depending on how you hand
     * Limine its requests — check what protocol revision you're targeting.
     */
    /* Define the program headers we want so the bootloader gives us the right */
    /* MMU permissions; this also allows us to exert more control over the linking */
    /* process. */
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}
SECTIONS
{
    /* the base address.
     *
     * -mcmodel=kernel means the kernel must live in the TOP 2 GB of the
     * address space. The conventional choice is 0xffffffff80000000. If this
     * address and your -mcmodel flag disagree, you get relocation truncation
     * errors at link time — or, worse, a kernel that loads and immediately
     * triple-faults.
     */
    . = 0xffffffff80000000;

    /* Move to the next memory page for .text */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    /* .text — code. Assign it to your executable PHDR. */
    .text : {
        *(.text .text.*)
    } :text

    /* Move to the next memory page for .rodata */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    /* .rodata — read-only data.
     * Your Limine request structures usually want to be somewhere the
     * bootloader will actually scan, and the requests must survive whatever
     * the linker does with unreferenced data. Look into KEEP() and
     * .requests / .requests_start_marker sections here.
     */

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata


    /* .data */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data : {
        *(.data .data.*)
    } :data

    /* TODO: .init_array — the array of global-constructor pointers.
     *
     * Milestone 1 requires you to walk this yourself. Define symbols marking
     * the start and end so C++ code can find the bounds. Watch out: the
     * linker can reorder/merge these; look at how SORT_BY_INIT_PRIORITY and
     * KEEP() are used in real scripts.
     */

    /* .bss — zero-initialised. */
    /* NOTE: .bss needs to be the last thing mapped to :data, otherwise lots of */
    /* unnecessary zeros will be written to the binary. */
    /* If you need, for example, .init_array and .fini_array, those should be placed */
    /* above this. */
    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /* Discard .note.* and .eh_frame* since they may cause issues on some hosts. */
    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}
