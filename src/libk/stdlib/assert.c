#include "../../include/libk/stdlib.h"
#include "../../include/go/go.h"

extern boolean _go_has_been_initialize;

void assertion_failure(char *exp, char *file, char *base, int line)
{
    if (_go_has_been_initialize) 
    {
        puts(output_bsp, "\n############ assertion failure ############\n");
        puts(output_bsp, "----> assert(");
        puts(output_bsp, exp);
        puts(output_bsp, ")\n");

        puts(output_bsp, "----> file: ");
        puts(output_bsp, file);
        putc(0,'\n');
        puts(output_bsp, "----> base: ");
        puts(output_bsp, base);
        putc(0,'\n');
        putsds(output_bsp, "----> line: ", line, "\n");
    }
    krnl_panic(L"#######################################\n");

}