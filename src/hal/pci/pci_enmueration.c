#include "pci.h"
#include "../../include/go/go.h"
#include "../../include/arch/mm.h"
#include "../../include/libk/stdlib.h"

Pci_bus_t* pci_enumerate_bus(uint16_t seg_grp, uint8_t bus);
Pci_dev_t* pci_enumerate_device(uint16_t seg_grp, uint8_t bus, uint8_t device);
Pci_func_t* pci_enumerate_function(uint16_t seg_grp, uint8_t bus, uint8_t device, uint8_t function);

Pci_func_t* pci_enumerate_function(uint16_t seg_grp, uint8_t bus, uint8_t device, uint8_t function)
{
    Pci_func_t* pci_func = NULL;
    dword vendor_id = 0;
    byte header_type = 0;
    byte secondary_bus = 0;
    Pci_bus_t *pci_secondary_bus = NULL;

    pci_read_config_dword(bus, device, function, 0x00, &vendor_id);
    if (vendor_id == 0xFFFFFFFF) {
        // No device present
        return NULL;
    }

    putsd(output_bsp, "Device found at bus ", bus);
    putsd(output_bsp, ", device ", device);
    putsds(output_bsp, ", function ", function, ".\n");

    pci_func = (Pci_func_t*)calloc(sizeof(*pci_func));

    /* 
     * Function's PCI configuration space is:
     *      Physical_Address = MMIO_Starting_Physical_Address + ((Bus) << 20 | Device << 15 | Function << 12)
     */
    uint64_t mmio_base_phys_addr = pci_seg_grp[seg_grp].base_addr
        + (bus << 20 | device << 15 | function << 12);

    uint64_t mmio_base_virt_addr = mm_set_mmio(mmio_base_phys_addr, 1);

    pci_func->config_space = (pci_config_space_t*)mmio_base_virt_addr;

    pci_read_config_byte(bus, device, function, 0xE, &header_type);
    header_type &= 0x7F;

    if (header_type == 0x01)
    {
        // It's a PCI-to-PCI bridge.
        pci_read_config_byte(bus, device, function, 0x19, &secondary_bus);

        pci_secondary_bus = pci_enumerate_bus(seg_grp, secondary_bus);

        if (pci_secondary_bus != NULL) {

            pci_func->type = PCItoPCI_Bridge;
            pci_func->func.Pci2PciBridge = pci_secondary_bus;

            // Add the secondary bus into pci_seg_grp.
            pci_secondary_bus->seg_grp = seg_grp;
            pci_secondary_bus->bus = secondary_bus;
            _list_push(&pci_seg_grp[seg_grp].buses, &pci_secondary_bus->bus_node);
        }
        pci_secondary_bus = NULL;
    }


    return pci_func;
}

Pci_dev_t* pci_enumerate_device(uint16_t seg_grp, uint8_t bus, uint8_t device) 
{
    Pci_dev_t* pci_dev = NULL;
    Pci_func_t* pci_func = NULL;

    byte function = 0;
    dword vendor_id = 0;
    byte header_type = 0;
    byte secondary_bus = 0;

    pci_read_config_dword(bus, device, function, 0x00, &vendor_id);

    if (vendor_id == 0xFFFFFFFF) {
        // No device present
        return NULL;
    }

    pci_dev = (Pci_dev_t*)calloc(sizeof(*pci_dev));

    for (int func_code = 0; func_code < MAX_PCI_FUNCTIONS; func_code++) {

        pci_func = pci_enumerate_function(seg_grp, bus, device, func_code);
        if (pci_func != NULL) {
            pci_dev->func_count++;
            pci_func->dev = pci_dev;            
            pci_func->func_code = func_code;
            _list_push(&pci_dev->functions, &pci_func->func_node);
        }
        pci_func = NULL;
    }


    return pci_dev;
}

Pci_bus_t* pci_enumerate_bus(uint16_t seg_grp, uint8_t bus) {

    Pci_bus_t* pci_bus = NULL;
    Pci_dev_t* pci_dev = NULL;

    for (uint8_t device = 0; device < MAX_PCI_DEVICES; device++) {

        pci_dev = pci_enumerate_device(seg_grp, bus, device);

        if (pci_dev != NULL)
        {
            if (pci_bus == NULL)
                pci_bus = (Pci_bus_t*)calloc(sizeof(*pci_bus)); 

            pci_bus->dev_count++;
            pci_dev->bus = pci_bus;
            pci_dev->dev = device;
            _list_push(&pci_bus->devs, &pci_dev->dev_node);

        }
        pci_dev = NULL;
    }

    return pci_bus;
}

void pci_enumerate_seg_grp(MCFG_ALLOCATION* mcfg) {

    Pci_bus_t* bus = NULL;

    while (mcfg->Reserved == 0) {

        pci_seg_grp[mcfg->PCIeSegmentGroup].base_addr = mcfg->BaseAddress;
        pci_seg_grp[mcfg->PCIeSegmentGroup].start_bus_num = mcfg->StartBusNumber;
        pci_seg_grp[mcfg->PCIeSegmentGroup].end_bus_num = mcfg->EndBusNumber;
        pci_seg_grp[mcfg->PCIeSegmentGroup].seg_grp = mcfg->PCIeSegmentGroup;

        // for (uint8_t i = mcfg->StartBusNumber; i < mcfg->EndBusNumber; i++) 
        // {
        bus = pci_enumerate_bus(mcfg->PCIeSegmentGroup, 0);
        if (bus != NULL) {
            bus->seg_grp = mcfg->PCIeSegmentGroup;
            bus->bus = 0;
            _list_push(&pci_seg_grp[mcfg->PCIeSegmentGroup].buses, &bus->bus_node);
        }
        bus = NULL;

        // }
        mcfg++;

    }

}
