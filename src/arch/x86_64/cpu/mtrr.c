#include "mtrr.h"
#include "cpu_features.h"
#include "cpu.h"
#include "../../../include/libk/stdlib.h"

const int nr_fix16k = 2;
const int nr_fix4k = 8;

uint8_t mtrr_phys_addr_size;
uint8_t mtrr_lin_addr_size;
uint8_t mtrr_guest_phys_addr_size;

mtrr_state_t mtrr_state;


/*
 * BIOS is expected to clear MtrrFixDramModEn bit, see for example
 * "BIOS and Kernel Developer's Guide for the AMD Athlon 64 and AMD
 * Opteron Processors" (26094 Rev. 3.30 February 2006), section
 * "13.2.1.2 SYSCFG Register": "The MtrrFixDramModEn bit should be set
 * to 1 during BIOS initialization of the fixed MTRRs, then cleared to
 * 0 for operation."
 */
// static inline void k8_check_syscfg_dram_mod_en(void)
// {
// 	u32 lo, hi;

// 	if (!((boot_cpu_data.x86_vendor == X86_VENDOR_AMD) &&
// 	      (boot_cpu_data.x86 >= 0x0f)))
// 		return;

// 	if (cc_platform_has(CC_ATTR_HOST_SEV_SNP))
// 		return;

// 	rdmsr(MSR_AMD64_SYSCFG, lo, hi);
// 	if (lo & K8_MTRRFIXRANGE_DRAM_MODIFY) {
// 		pr_err(FW_WARN "MTRR: CPU %u: SYSCFG[MtrrFixDramModEn]"
// 		       " not cleared by BIOS, clearing this bit\n",
// 		       smp_processor_id());
// 		lo &= ~K8_MTRRFIXRANGE_DRAM_MODIFY;
// 		mtrr_wrmsr(MSR_AMD64_SYSCFG, lo, hi);
// 	}
// }

void get_mtrr_state(void)
{
    uint64_t value;
    uint64_t* fix_reg_ptr;
    uint64_t* phys_reg_ptr;

    fix_reg_ptr = (uint64_t*)mtrr_state.fixed_range;
    phys_reg_ptr = (uint64_t*)mtrr_state.phys_desc;

    value = rdmsr(IA32_MTRRCAP);
    mtrr_state.vcnt = value & MTRRCAP_VCNT;
    mtrr_state.wc = !!(value & MTRRCAP_WC);
    mtrr_state.fix = !!(value & MTRRCAP_FIX);

    value = rdmsr(IA32_MTRR_DEF_TYPE);
    mtrr_state.mtrr_enable = !!(value & MTRR_DEF_ENABLE);
    mtrr_state.fixed_range_enable = !!(value & MTRR_DEF_FE);
    mtrr_state.type = value & MTRR_DEF_TYPE;

    if (mtrr_state.fix) 
    {
        value = rdmsr(IA32_MTRR_FIX64K_00000);
        fix_reg_ptr[0] = value;

        for (int i = 0; i < nr_fix16k; i++) {
            value = rdmsr(IA32_MTRR_FIX16K(i));
            fix_reg_ptr[i + 1] = value;
        }
        for (int i = 0; i < nr_fix4k; i++) {
            value = rdmsr(IA32_MTRR_FIX4K(i));
            fix_reg_ptr[3 + i] = value;
        }
    }

    for (int i = 0; i < mtrr_state.vcnt; i++) {
        value = rdmsr(IA32_MTRR_PHYSBASE(i));
        phys_reg_ptr[(i) << 1] = value;
        value = rdmsr(IA32_MTRR_PHYSMASK(i));
        phys_reg_ptr[((i) << 1) + 1] = value;
    }

}

void mtrr_init(void)
{
    boolean support_mtrr = cpu_feature_support(X86_FEATURE_MTRR);
    uint64_t value;
    uint32_t eax, ebx, ecx, edx;

    if (support_mtrr) 
    {
    
        cpuid(CPUID_MAX_PHYS_ADDR, &eax, &ebx, &ecx, &edx);
        mtrr_phys_addr_size = eax & 0xff;
        mtrr_lin_addr_size = (eax >> 8) & 0xff;
        mtrr_guest_phys_addr_size = (eax >> 16) & 0xff;
    
        get_mtrr_state();

    }
    if (!memcmp(cpu_feat_id.vendor_id, CPUID_VENDOR_AMD, sizeof(CPUID_VENDOR_AMD)))
    {
        value = rdmsr(AMD_MSR_SYSCFG_REG);
        if (value & AMD_MSR_SYSCFG_MtrrFixDramModEn)
        {
            value &= 0xFFFFFFFFFFF7FFFFUL;
            wrmsr(AMD_MSR_SYSCFG_REG, value);
        }
    }
}
