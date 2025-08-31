#include "pci.h"
#include "../../include/arch/io.h"

void pci_read_config_byte(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset, unsigned char *val)
{
    outl((0x80000000 | ((bus)<<16) |((dev)<<11) | ((func)<<8) | (offset & ~0x3)), PCI_CFG_CTRL);
    *val = inl(PCI_CFG_DATA) >> ((offset & 3) * 8);
}

void pci_read_config_word(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset, unsigned short *val)
{
    outl((0x80000000 | ((bus)<<16) |((dev)<<11) | ((func)<<8) | (offset & ~0x3)), PCI_CFG_CTRL);
    *val = inl(PCI_CFG_DATA) >> ((offset & 3) * 8);
}

void pci_read_config_dword(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset, unsigned int *val)
{
    outl((0x80000000 | ((bus)<<16) |((dev)<<11) | ((func)<<8) | (offset)), PCI_CFG_CTRL);
    *val = inl(PCI_CFG_DATA);
}
