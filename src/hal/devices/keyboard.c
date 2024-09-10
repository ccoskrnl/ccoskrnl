#include "../../include/types.h"
#include "../../include/machine_info.h"

#include "../../include/go/go.h"
#include "../../include/go/window.h"

#include "../../include/arch/io.h"
#include "../../include/arch/lib.h"

// 	Read/Write	Data Port
//  The Data Port (IO Port 0x60) is used for reading data that 
//  was received from a PS/2 device or from the PS/2 controller 
//  itself and writing data to a PS/2 device or to the PS/2 controller itself.
#define PS2_CTR_IO_DATA_PORT                        0x60

//  Read	Status Register
#define PS2_CTR_IO_STATUS_REG                       0x64

//  Write	Command Register
#define PS2_CTR_IO_CMD_REG                          0x64

// Default interval in milliseconds
static int key_repeat_interval = 30;

static boolean caps_lock = false;
static boolean scroll_lock = false;
static boolean number_lock = false;
static boolean ext_code = false;

static go_blt_pixel_t color = { 0, 0, 0, 0};

// Keyboard ISR
void keyboard_isr() 
{
    wch_t which_key = 0;
    // Read the scancode from the keyboard controller
    uint8_t scancode = inb(PS2_CTR_IO_DATA_PORT);

    switch (scancode) 
    {
        case 0x1E:
            which_key = 'a';
            break;
        default:
            break;
    }

    if (which_key != 0)
        _go_cpu_output_window[0]->PutChar(_go_cpu_output_window[0], which_key, color, true);

    send_eoi();
}