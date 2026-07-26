/* Linker script for the hobby-os kernel (Limine, x86_64).
 *
 * This file is yours to work out — it's core milestone-1 material, not
 * plumbing. Skeleton and decision points only.
 *
 * Read first:
 *   https://wiki.osdev.org/Linker_Scripts
 *   https://wiki.osdev.org/Limine_Bare_Bones   (its linker.ld is the model)
 */

OUTPUT_FORMAT(elf64-x86-64)
ENTRY(/* TODO: entry symbol — whatever your boot.S / main exports */)

PHDRS
{
    /* TODO: declare segments. Limine wants separate loadable segments so it
     * can map them with the right permissions (text RX, rodata R, data RW).
     * Each needs PT_LOAD and a FLAGS(...) bitmask: 1=X, 2=W, 4=R.
     *
     * You may also need a PT_NOTE-style entry depending on how you hand
     * Limine its requests — check what protocol revision you're targeting.
     */
}

SECTIONS
{
    /* TODO: the base address.
     *
     * -mcmodel=kernel means the kernel must live in the TOP 2 GB of the
     * address space. The conventional choice is 0xffffffff80000000. If this
     * address and your -mcmodel flag disagree, you get relocation truncation
     * errors at link time — or, worse, a kernel that loads and immediately
     * triple-faults.
     */
    . = /* TODO */ ;

    /* TODO: .text — code. Assign it to your executable PHDR. */

    /* TODO: .rodata — read-only data.
     *
     * Your Limine request structures usually want to be somewhere the
     * bootloader will actually scan, and the requests must survive whatever
     * the linker does with unreferenced data. Look into KEEP() and
     * .requests / .requests_start_marker sections here.
     */

    /* TODO: .data */

    /* TODO: .init_array — the array of global-constructor pointers.
     *
     * Milestone 1 requires you to walk this yourself. Define symbols marking
     * the start and end so C++ code can find the bounds. Watch out: the
     * linker can reorder/merge these; look at how SORT_BY_INIT_PRIORITY and
     * KEEP() are used in real scripts.
     */

    /* TODO: .bss — zero-initialised. NOLOAD. You need start/end symbols so
     * early boot code can zero it before any C++ runs. */

    /DISCARD/ : {
        /* TODO: discard what you don't want — .comment, .eh_frame, .note.*
         * Careful: dropping .eh_frame is fine with -fno-exceptions, but be
         * deliberate about it rather than cargo-culting.
         */
    }
}
