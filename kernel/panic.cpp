#include <console.hpp>
#include <panic.hpp>

#include <arch/x86_64/cpu.hpp>

namespace kernel {

using console::print;

void panic(const char* message) {
    print("PANIC: ");
    println(message);
    x86_64::cpu::halt();
}

} // namespace kernel
