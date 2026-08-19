#ifndef KERNEL_PANIC_HPP
#define KERNEL_PANIC_HPP

namespace kernel {

[[noreturn]] void panic(const char* message);

}
using kernel::panic;

#endif
