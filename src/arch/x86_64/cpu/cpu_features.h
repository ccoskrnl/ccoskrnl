#ifndef __X86_64_CPU_FEATURES_H__
#define __X86_64_CPU_FEATURES_H__

#include "../../../include/types.h"



// Vendor strings from CPUs.
#define CPUID_VENDOR_AMD           "AuthenticAMD"
#define CPUID_VENDOR_AMD_OLD       "AMDisbetter!" // Early engineering samples of AMD K5 processor
#define CPUID_VENDOR_INTEL         "GenuineIntel"
#define CPUID_VENDOR_VIA           "VIA VIA VIA "
#define CPUID_VENDOR_TRANSMETA     "GenuineTMx86"
#define CPUID_VENDOR_TRANSMETA_OLD "TransmetaCPU"
#define CPUID_VENDOR_CYRIX         "CyrixInstead"
#define CPUID_VENDOR_CENTAUR       "CentaurHauls"
#define CPUID_VENDOR_NEXGEN        "NexGenDriven"
#define CPUID_VENDOR_UMC           "UMC UMC UMC "
#define CPUID_VENDOR_SIS           "SiS SiS SiS "
#define CPUID_VENDOR_NSC           "Geode by NSC"
#define CPUID_VENDOR_RISE          "RiseRiseRise"
#define CPUID_VENDOR_VORTEX        "Vortex86 SoC"
#define CPUID_VENDOR_AO486         "MiSTer AO486"
#define CPUID_VENDOR_AO486_OLD     "GenuineAO486"
#define CPUID_VENDOR_ZHAOXIN       "  Shanghai  "
#define CPUID_VENDOR_HYGON         "HygonGenuine"
#define CPUID_VENDOR_ELBRUS        "E2K MACHINE "
 
// Vendor strings from hypervisors.
#define CPUID_VENDOR_QEMU          "TCGTCGTCGTCG"
#define CPUID_VENDOR_KVM           " KVMKVMKVM  "
#define CPUID_VENDOR_VMWARE        "VMwareVMware"
#define CPUID_VENDOR_VIRTUALBOX    "VBoxVBoxVBox"
#define CPUID_VENDOR_XEN           "XenVMMXenVMM"
#define CPUID_VENDOR_HYPERV        "Microsoft Hv"
#define CPUID_VENDOR_PARALLELS     " prl hyperv "
#define CPUID_VENDOR_PARALLELS_ALT " lrpepyh vr " // Sometimes Parallels incorrectly encodes "prl hyperv" as "lrpepyh vr" due to an endianness mismatch.
#define CPUID_VENDOR_BHYVE         "bhyve bhyve "
#define CPUID_VENDOR_QNX           " QNXQVMBSQG "







// Base address of Local APIC
#define IA32_APIC_BASE_MSR                                                  0x1B
// TSC Frequency Clock Counter (R/Write to clear)
#define IA32_MPERF                                                          0xE7
// Actual Performance Clock Counter (R/Write to clear)
#define IA32_APERF                                                          0xE8
// This field indicates the intended scaleable bus clock speed 
// for processors based on Intel Core microarchitecture.
#define MSR_FSB_FREQ                                                        0xCD

// TSC Ratio MSR
#define MSR_TSC_RADIO                                                       0xC0000104

// MSR for Core current operating frequency in MHz.
// #define MSR_P_STATE_0_FREQ                                                  0xC0010064

uint64_t rdmsr(uint32_t msr);
void wrmsr(uint32_t msr, uint64_t value);









// Largest Standard Function Number
#define CPUID_LEFT_LARGEST_STD_FUNC_NUM                                     0x0

// Miscellaneous feature identifiers
#define CPUID_LEFT_FEATURE_IDENTIFIERS                                      0x1

// Structured Extended Feature Flags Enumeration Leaf
#define CPUID_LEFT_STRUCTURED_EX_FEATURE_FLAGS                              0x7

#define CPUID_LEFT_THERMAL_AND_POWER_MANAGEMENT                             0x6

// Time Stamp Counter and Nominal Core Crystal Clock Information Leaf
#define CPUID_LEFT_TSC                                                      0x15

// Processor Frequency Information
#define CPUID_LEFT_PROCESSOR_FREQUENCY_INFO                                 0x16

// Processor Brand String
#define CPUID_LEFT_PROCESSOR_BRAND                                          0x80000002

// Advanced Power Management Information
#define CPUID_EX_LEFT_APMF                                                  0x80000007

void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);

void cpuid_ex(uint32_t leaf, uint32_t fn, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);








// CPU functions
uint64_t rdtsc();









// CPU Vendor.
extern char cpu_vendor_id[16];

// Current Machine cpu brand.
extern char cpu_brand_string[49];  // 48 characters + null terminator


extern boolean support_rdrand;
// AVX2. Supports Intel® Advanced Vector Extensions 2 (Intel® AVX2) if 1.
extern boolean support_avx2;
// AVX instruction support
extern boolean support_avx;
// XSAVE (and related) instructions are enabled.
extern boolean support_osxsave;
// XSAVE (and related) instructions are supported by hardware. .
extern boolean support_xsave;
// AES instruction support.
extern boolean support_aes;
// SSE4.2 instruction support.
extern boolean support_sse42;
// SSE4.1 instruction support. 
extern boolean support_sse41;
// CMPXCHG16B instruction
extern boolean support_cmpxchg16b;
// FMA instruction support.
extern boolean support_fma;
// supplemental SSE3 instruction support.
extern boolean support_ssse3;
// SSE3 instruction support.
extern boolean support_sse3;

#endif
