#include "../../include/types.h"
#include "../../include/machine_info.h"

#include "../../include/go/go.h"

#include "../../include/arch/io.h"
#include "../../include/arch/lib.h"


// Keyboard ISR
void keyboard_isr() {

    // Read the scancode from the keyboard controller
    uint8_t scancode = inb(0x60);

    putsxs(0, "Scancode: ", scancode, "\n");

    send_eoi();
}