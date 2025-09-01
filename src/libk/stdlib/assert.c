#include "../../include/libk/stdlib.h"
#include "../../include/go/go.h"

extern boolean _go_has_been_initialize;

void assertion_failure(char *exp, char *file, char *base, int line)
{
    if (_go_has_been_initialize) 
    {
        puts(bsp_window, "\n############ assertion failure ############\n");
        puts(bsp_window, "----> assert(");
        puts(bsp_window, exp);
        puts(bsp_window, ")\n");

        puts(bsp_window, "----> file: ");
        puts(bsp_window, file);
        putc(0,'\n');
        puts(bsp_window, "----> base: ");
        puts(bsp_window, base);
        putc(0,'\n');
        putsds(bsp_window, "----> line: ", line, "\n");
    }
    krnl_panic(L"#######################################\n");

}