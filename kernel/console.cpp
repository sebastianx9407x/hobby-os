#include <arch/x86_64/serial.hpp>

namespace kernel::console {

void print(const char* string) {
    if (string == nullptr) {
        print("(null)");
        return;
    }

    for (const char* p = string; *p != '\0'; ++p) {
        kernel::x86_64::serial::writeSerial(*p);
    }
}

void println(const char* string) {
    print(string);
    kernel::x86_64::serial::writeSerial('\r');
    kernel::x86_64::serial::writeSerial('\n');
}

} // namespace kernel::console
