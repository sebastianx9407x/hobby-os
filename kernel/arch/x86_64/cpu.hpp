#ifndef KERNEL_ARCH_X86_64_CPU_HPP
#define KERNEL_ARCH_X86_64_CPU_HPP

namespace kernel::x86_64::cpu {

inline void cli() { __asm__ volatile("cli" : : : "memory"); } // Disable interrupts

inline void sti() { __asm__ volatile("sti" : : : "memory"); } // Enable interrupts

inline void hlt() { __asm__ volatile("hlt"); }

[[noreturn]] inline void halt() {
    cli();
    for (;;) {
        hlt();
    }
}

} // namespace kernel::x86_64::cpu

#endif
