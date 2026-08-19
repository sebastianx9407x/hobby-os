#ifndef KERNEL_CONSOLE_HPP
#define KERNEL_CONSOLE_HPP

namespace kernel::console {

void print(const char* string);

void println(const char* string);

} // namespace kernel::console

using kernel::console::println;

#endif
