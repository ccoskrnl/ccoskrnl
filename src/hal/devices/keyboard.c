#include "../../include/types.h"
#include "../../include/go/go.h"
#include "../../include/arch/io.h"
#include "../../include/libk/string.h"
#include "../../include/machine_info.h"

// Keyboard ISR
void keyboard_isr() {
    uint8_t scancode = inb(0x60); // Read the scancode from the keyboard controller
    // Process the scancode (e.g., convert to ASCII, handle special keys)
}