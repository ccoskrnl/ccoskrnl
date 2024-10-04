#include "../../../include/types.h"

boolean get_intr_state()
{
    __asm__ volatile(
        "pushfq\n"
        "popq %%rax\n"
        "shrq $9, %%rax\n"
        "andq $1, %%rax\n"
        :
        :
        : "rax"
    );
}

boolean intr_disable()
{
    __asm__ volatile (
        "pushfq\n"
        "cli\n"
        "popq %%rax\n"
        "shrq $9, %%rax\n"
        "andq $1, %%rax\n"
        :
        :
        : "rax"
    );
}

void set_intr_state(boolean state)
{
    if (state)
        __asm__ volatile ("sti\n");
    else
        __asm__ volatile ("cli\n");
}