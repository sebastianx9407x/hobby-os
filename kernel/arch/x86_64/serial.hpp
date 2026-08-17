#ifndef KERNEL_ARCH_X86_64_SERIAL
#define KERNEL_ARCH_X86_64_SERIAL
#include <stdint.h>

// https://wiki.osdev.org/Serial_Ports

namespace kernel::x86_64::serial {

bool init_serial();

uint8_t readSerial();

void writeSerial(const uint8_t byte);

[[nodiscard]] bool tryRead(uint8_t& byte);

[[nodiscard]] bool tryWrite(const uint8_t byte);

void print(const char* string);

void println(const char* string);

} // namespace kernel::x86_64::serial
#endif
