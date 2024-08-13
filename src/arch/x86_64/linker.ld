/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(krnl_start)

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* Begin putting sections at 0xfffff00000001000, a conventional place for kernels to be
	   loaded at by the ccloader. */
	. = 0xfffff00000001000;
	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	   
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
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

	/* The compiler may produce other sections, put them in the proper place in
	   in this file, if you'd like to include them in the final kernel. */
}
