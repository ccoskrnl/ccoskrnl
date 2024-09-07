#include "../../include/libk/stdlib.h"
#include "../../include/go/go.h"

extern boolean _go_has_been_initialize;

void assertion_failure(char *exp, char *file, char *base, int line)
{
    if (_go_has_been_initialize) 
    {
        puts(0, "\n############ assertion failure ############\n");
        puts(0, "----> assert(");
        puts(0, exp);
        puts(0, ")\n");

        puts(0, "----> file: ");
        puts(0, file);
        putc(0,'\n');
        puts(0, "----> base: ");
        puts(0, base);
        putc(0,'\n');
        putsds(0, "----> line: ", line, "\n");
    }
    krnl_panic(L"#######################################\n");

}