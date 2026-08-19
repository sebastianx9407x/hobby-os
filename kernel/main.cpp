#include <limine.h>

#include <arch/x86_64/serial.hpp>

#include <console.hpp>
#include <panic.hpp>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.
__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests"))) static volatile struct limine_framebuffer_request
    framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0, .response = nullptr};

__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
extern "C" void kmain(void) {
    kernel::x86_64::serial::init_serial();
    println("QEMU has entered kmain");

    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        panic("kmain: LIMINE_BASE_REVISION_SUPPORTED == false");
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == nullptr || framebuffer_request.response->framebuffer_count < 1) {
        panic("kmain: frame_buffer not accessed");
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];

    // Print a nice pattern to screen as an example.
    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    volatile uint32_t* fb_ptr = static_cast<volatile uint32_t*>(framebuffer->address);
    for (size_t y = 0; y < framebuffer->height; y++) {
        for (size_t x = 0; x < framebuffer->width; x++) {
            uint32_t nX                              = x * 255 / framebuffer->width;
            uint32_t nY                              = y * 255 / framebuffer->height;
            fb_ptr[y * (framebuffer->pitch / 4) + x] = (nY << 8) | nX;
        }
    }

    println("kmain: first frame_buffer has been read");

    // We're done, just hang...
    panic("Nothing left to do, end of kmain");
}
