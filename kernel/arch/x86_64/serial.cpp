#include <arch/x86_64/io.hpp>
#include <arch/x86_64/serial.hpp>
#include <stdint.h>

// https://wiki.osdev.org/Serial_Ports

namespace kernel::x86_64::serial {

enum class Port : uint16_t { COM1 = 0x3F8 };

enum class Register : uint16_t {
    DataBuffer      = 0,
    InterruptEnable = 1,
    FifoControl     = 2,
    LineControl     = 3,
    ModemControl    = 4,
    LineStatus      = 5,
};

namespace InterruptEnable {
constexpr uint8_t None = 0x00;
// constexpr uint8_t DataAvailable     = 0x01;
// constexpr uint8_t TransmitterEmpty  = 0x02;
// constexpr uint8_t LineStatusChange  = 0x04;
// constexpr uint8_t ModemStatusChange = 0x08;
} // namespace InterruptEnable

namespace FifoControl {
constexpr uint8_t Enable        = 0x01;
constexpr uint8_t ClearReceive  = 0x02;
constexpr uint8_t ClearTransmit = 0x04;
// constexpr uint8_t DmaMode        = 0x08;
// constexpr uint8_t Trigger1Byte   = 0x00;
// constexpr uint8_t Trigger4Bytes  = 0x40;
// constexpr uint8_t Trigger8Bytes  = 0x80;
constexpr uint8_t Trigger14Bytes = 0xC0;
} // namespace FifoControl

namespace LineControl {
// constexpr uint8_t DataBits5 = 0x00;
// constexpr uint8_t DataBits6 = 0x01;
// constexpr uint8_t DataBits7 = 0x02;
constexpr uint8_t DataBits8 = 0x03;
// constexpr uint8_t TwoStopBits  = 0x04;
// constexpr uint8_t ParityEnable = 0x08;
// constexpr uint8_t ParityEven   = 0x10;
// constexpr uint8_t ParityStick  = 0x20;
// constexpr uint8_t BreakEnable  = 0x40;
constexpr uint8_t Dlab = 0x80;
} // namespace LineControl

namespace ModemControl {
constexpr uint8_t Dtr      = 0x01;
constexpr uint8_t Rts      = 0x02;
constexpr uint8_t Out1     = 0x04;
constexpr uint8_t Out2     = 0x08;
constexpr uint8_t Loopback = 0x10;
} // namespace ModemControl

namespace LineStatus {
constexpr uint8_t DataReady = 0x01;
// constexpr uint8_t OverrunError   = 0x02;
// constexpr uint8_t ParityError    = 0x04;
// constexpr uint8_t FramingError   = 0x08;
// constexpr uint8_t BreakInterrupt = 0x10;
constexpr uint8_t TransmitEmpty = 0x20;
// constexpr uint8_t TransmitterIdle = 0x40;
// constexpr uint8_t FifoError       = 0x80;
} // namespace LineStatus

constexpr uint8_t BaudDivisorLow  = 0x01;
constexpr uint8_t BaudDivisorHigh = 0x00;

constexpr uint8_t LoopbackTestByte = 0xAE;

constexpr uint16_t registerOf(Port port, Register reg) {
    return static_cast<uint16_t>(port) + static_cast<uint16_t>(reg);
}

bool init_serial() {
    outb(registerOf(Port::COM1, Register::InterruptEnable), InterruptEnable::None); // Disable all interrupts
    outb(registerOf(Port::COM1, Register::LineControl), LineControl::Dlab);   // Enable DLAB (set baud rate divisor)
    outb(registerOf(Port::COM1, Register::DataBuffer), BaudDivisorLow);       // Set divisor to 1 (lo byte) 115200 baud
    outb(registerOf(Port::COM1, Register::InterruptEnable), BaudDivisorHigh); //                  (hi byte)
    outb(registerOf(Port::COM1, Register::LineControl), LineControl::DataBits8); // 8 bits, no parity, one stop bit 8N1
    outb(
        registerOf(Port::COM1, Register::FifoControl),
        FifoControl::Enable | FifoControl::ClearReceive | FifoControl::ClearTransmit | FifoControl::Trigger14Bytes
    ); // Enable FIFO, clear them, with 14-byte threshold
    outb(
        registerOf(Port::COM1, Register::ModemControl),
        ModemControl::Dtr | ModemControl::Rts | ModemControl::Out2
    ); // IRQs enabled, RTS/DSR set
    outb(
        registerOf(Port::COM1, Register::ModemControl),
        ModemControl::Rts | ModemControl::Out1 | ModemControl::Out2 | ModemControl::Loopback
    );                                                                    // Set in loopback mode, test the serial chip
    outb(registerOf(Port::COM1, Register::DataBuffer), LoopbackTestByte); // Test serial chip

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(registerOf(Port::COM1, Register::DataBuffer)) != LoopbackTestByte) {
        return false;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(
        registerOf(Port::COM1, Register::ModemControl),
        ModemControl::Dtr | ModemControl::Rts | ModemControl::Out1 | ModemControl::Out2
    );
    return true;
}

bool serialReceived() { return inb(registerOf(Port::COM1, Register::LineStatus)) & LineStatus::DataReady; }

uint8_t readSerial() {
    while (!serialReceived())
        ;

    return inb(registerOf(Port::COM1, Register::DataBuffer));
}

bool isTransmitBufferEmpty() { return inb(registerOf(Port::COM1, Register::LineStatus)) & LineStatus::TransmitEmpty; }

void writeSerial(const uint8_t byte) {
    while (!isTransmitBufferEmpty())
        ;

    outb(registerOf(Port::COM1, Register::DataBuffer), byte);
}

bool tryRead(uint8_t& byte) {
    if (serialReceived()) {
        byte = inb(registerOf(Port::COM1, Register::DataBuffer));
        return true;
    }
    return false;
}

bool tryWrite(const uint8_t byte) {
    if (isTransmitBufferEmpty()) {
        outb(registerOf(Port::COM1, Register::DataBuffer), byte);
        return true;
    }
    return false;
}

void print(const char* string) {
    if (string == nullptr) {
        print("(null)");
        return;
    }

    for (const char* p = string; *p != '\0'; ++p) {
        writeSerial(*p);
    }
}

void println(const char* string) {
    print(string);
    writeSerial('\r');
    writeSerial('\n');
}

} // namespace kernel::x86_64::serial
