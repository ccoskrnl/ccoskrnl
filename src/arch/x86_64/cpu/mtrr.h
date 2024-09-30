#ifndef __X86_MTRR_H__
#define __X86_MTRR_H__

#include "../../../include/types.h"

// This register describes memory range types and count supported by the implementation. 
// The register is read-only.
typedef struct _IA32_mtrr_cap
{
    // Count of variable-size MTRRs supported
    uint64_t vcnt : 8;

    // All fixed-size MTRRs are available
    uint64_t fix : 1;
    uint64_t reserved0 : 1;

    // Write-combining type is available
    uint64_t wc : 1;
    uint64_t reserved1 : 53;

} __attribute__ ((packed)) IA32_mtrr_cap_t;

extern IA32_mtrr_cap_t mtrr_cap;


// Describes the default type used for physical addresses which are outside of any 
// configured memory ranges, as well as whether MTRRs and fixed ranges are enabled.
typedef struct _IA32_mtrr_def_type
{
    // Default memory type
    uint64_t type : 8;

    uint64_t reserved0 : 2;

    // Default memory type
    uint64_t fixed_range_enable : 1;

    // Default memory type
    uint64_t mtrr_enable : 1;

    uint64_t reserved1 : 52;

} __attribute__ ((packed)) IA32_mtrr_def_type_t;

extern IA32_mtrr_def_type_t mtrr_def_type;


// For example, to enter a base address of 2 MBytes (200000H) in the IA32_MTRR_PHYSBASE3 register, the 12 least
// significant bits are truncated and the value 000200H is entered in the PhysBase field. The same operation must be 
// performed on mask values. For example, to map the address range from 200000H to 3FFFFFH (2 MBytes to 4 
// MBytes), a mask value of FFFE00000H is required. Again, the 12 least-significant bits of this mask value are trun
// -cated, so that the value entered in the PhysMask field of IA32_MTRR_PHYSMASK3 is FFFE00H. This mask is chosen 
// so that when any address in the 200000H to 3FFFFFH range is AND’d with the mask value, it will return the same 
// value as when the base address is AND’d with the mask value (which is 200000H).

// TODO Implement MTRR Maintenance Programming Interface

// MAXPHYADDR: The bit position indicated by MAXPHYADDR depends on the maximum
// physical address range supported by the processor. It is reported by CPUID leaf
// function 80000008H. If CPUID does not support leaf 80000008H, the processor
// supports 36-bit physical address size, then bit PhysMask consists of bits 35:12, and
// bits 63:36 are reserved.

// Base register
typedef struct _IA32_mtrr_phys_base
{
    // Specifies the memory type for the range
    uint64_t type : 8;

    uint64_t reserved0 : 4;

    // Specifies the base address of the address range. 
    // This 24-bit value, in the case where MAXPHYADDR is 36 bits, is extended by 12 bits at the low end to form the 
    // base address (this automatically aligns the address on a 4-KByte boundary).
    uint64_t phys_base : 52;

} __attribute__ ((packed)) IA32_mtrr_phys_base_t;

// Mask register
typedef struct _IA32_mtrr_phys_mask
{
    uint64_t reserved0 : 11;
    uint64_t valid : 1;

    // Specifies a mask (24 bits if the maximum physical 
    // address size is 36 bits, 28 bits if the maximum physical address size is 40 bits).
    uint64_t phys_mask : 52;

} __attribute__ ((packed)) IA32_mtrr_phys_mask_t;

typedef struct _mtrr_state
{
    uint8_t fixed_range[11 * 8];
    // Count of variable-size MTRRs supported
    uint8_t vcnt;
    // All fixed-size MTRRs are available
    boolean fix;
    // Write-combining type is available
    boolean wc;
    
    // Default memory type
    uint8_t type;
    // Default memory type
    boolean fixed_range_enable;
    // Default memory type
    boolean mtrr_enable;

    struct {
        IA32_mtrr_phys_base_t base;
        IA32_mtrr_phys_mask_t mask;
    } phys_desc[256];

} mtrr_state_t;


#define AMD_MSR_SYSCFG_REG                                                  0xc0010010
#define AMD_MSR_SYSCFG_MtrrFixDramModEn                                     0x80000

#define CPUID_MAX_PHYS_ADDR                                                 0x80000008


// IA32_MTRRCAP (MSR 0xFE)
#define IA32_MTRRCAP                                                        0xFE
#define MTRRCAP_SMRR                                                        (1 << 11)
#define MTRRCAP_WC                                                          (1 << 10)
#define MTRRCAP_FIX                                                         (1 << 8)
#define MTRRCAP_VCNT                                                        (0xFF)

// IA32_MTRRdefType (MSR 0x2FF)
#define IA32_MTRR_DEF_TYPE                                                  0x2FF
#define MTRR_DEF_ENABLE                                                     (1 << 11)
#define MTRR_DEF_FE                                                         (1 << 10)
#define MTRR_DEF_TYPE                                                       (0xFF)

#define IA32_MTRR_PHYSBASE0                                                 0x200
#define IA32_MTRR_PHYSMASK0                                                 0x201
#define IA32_MTRR_PHYSBASE(CNT)                                             (IA32_MTRR_PHYSBASE0 + (CNT << 1))
#define IA32_MTRR_PHYSMASK(CNT)                                             (IA32_MTRR_PHYSMASK0 + (CNT << 1))

#define IA32_MTRR_FIX64K_00000                                              0x250

#define IA32_MTRR_FIX16K_80000                                              0x258
#define IA32_MTRR_FIX16K_A0000                                              0x259
#define IA32_MTRR_FIX16K(CNT)                                               (IA32_MTRR_FIX16K_80000 + (CNT))

#define IA32_MTRR_FIX4K_C0000                                               0x268
#define IA32_MTRR_FIX4K(CNT)                                                (IA32_MTRR_FIX4K_C0000 + (CNT))

extern const int nr_fix16k;
extern const int nr_fix4k;
extern mtrr_state_t mtrr_state;

void mtrr_bsp_init(void);

#endif
