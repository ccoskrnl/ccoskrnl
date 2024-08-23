/* 	We specifie the output format of the binary. Here, it is set to elf64-x86-64, 
	which is the 64-bit ELF format for x86-64 architecture. */
OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")

/* 	This specifies the target architecture for the output file. In this case, 
	it is i386:x86-64, indicating a 64-bit x86 architecture. */
OUTPUT_ARCH(i386:x86-64)

/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(krnl_start)

/* We tell linker where it should search for libraries. */
SEARCH_DIR("../reflibs/libm.a")

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* 	Begin putting sections at 0xfffff00000001000, ccloader will mapping kernel space into
		higher address space during ccloader second phrase. And in end, it will use jmp
		instruction to jump here, then kernel will take over cpu's control. */
	. = 0xfffff00000001000;

	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
		*(.text.*)
	}

	/*	Actually, we don't have to use gcc_except_table, but libm.a references it. We collect and merga 
		these sections into one section to obtain a clearer and simple elf structure. */
	.gcc_except_table BLOCK(4K): ALIGN(4K)
    {
        *(.gcc_except_table)
        *(.gcc_except_table.*)
    }

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
		*(.rodata.*)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data .data.*)

        /* We place the .bss section in .data as .bss is of type SHT_NOBITS by default but we 
           need its symbols to be in a section of type SHT_PROGBITS so that FILESIZE  
           MEMSIZE for each LOAD segment. */
        /* The .bss output section of an ELF executable (or shared lib) actually consists of 
           symbols that are either in the COMMON section or the `.bss` section of object files.
           This can also be verified by looking at the standard linker script for Linux
           programs. */
		*(COMMON)
		*(.bss .bss.*)
	}

}
