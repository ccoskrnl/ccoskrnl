#include "cpu.h"
#include "cpu_features.h"


/**
 * CPU Feature Identifer.
 */
cpu_feat_id_t cpu_feat_id;

// rdrand instruction to generate a random number.
static boolean support_rdrand;
// AVX2. Supports Intel® Advanced Vector Extensions 2 (Intel® AVX2) if 1.
static boolean support_avx2;
// AVX instruction support
static boolean support_avx;
// XSAVE (and related) instructions are enabled.
static boolean support_osxsave;
// XSAVE (and related) instructions are supported by hardware. .
static boolean support_xsave;
// AES instruction support.
static boolean support_aes;
// SSE4.2 instruction support.
static boolean support_sse42;
// SSE4.1 instruction support. 
static boolean support_sse41;
// CMPXCHG16B instruction
static boolean support_cmpxchg16b;
// FMA instruction support.
static boolean support_fma;
// supplemental SSE3 instruction support.
static boolean support_ssse3;
// SSE3 instruction support.
static boolean support_sse3;
// MTRR
static boolean support_mtrr;

static void get_cpu_info(cpu_feat_id_t* p_cpu_feat_id)
{
    uint32_t eax, ebx, ecx, edx;
    char* processor_name = cpu_feat_id.brand;

    // Get CPU Vendor
    cpuid(CPUID_LEFT_LARGEST_STD_FUNC_NUM, &eax, &ebx, &ecx, &edx);
    {
        char uc;
        int i = 3;
        do
        {
            uc = (char)(ebx >> (i * 8));
            cpu_feat_id.vendor_id[i] = uc;

        } while (i-- >= 0);
        i = 7;
        do
        {
            uc = (char)(edx >> (i * 8));
            cpu_feat_id.vendor_id[i] = uc;

        } while (i-- > 4);
        i = 11;
        do
        {
            uc = (char)(ecx >> (i * 8));
            cpu_feat_id.vendor_id[i] = uc;

        } while (i-- > 8);

    }

    // Get CPU Brand String
    for (size_t i = 0; i < 3; i++)
    {
        cpuid(CPUID_LEFT_PROCESSOR_BRAND + i, &eax, &ebx, &ecx, &edx);
        *(uint32_t*)processor_name = eax;
        processor_name += 4;
        *(uint32_t*)processor_name = ebx;
        processor_name += 4;
        *(uint32_t*)processor_name = ecx;
        processor_name += 4;
        *(uint32_t*)processor_name = edx;
        processor_name += 4;
    }

    cpuid(CPUID_LEFT_FEATURE_IDENTIFIERS, &eax, &ebx, &ecx, &edx);

    p_cpu_feat_id->stepping = eax & 0xf;

    /**
     * Model is an 8-bit value and is defined as: Model[7:0] = {ExtendedModel[3:0],BaseModel[3:0]}
     */
    p_cpu_feat_id->model = (((eax >> 4) & 0xf) | ((eax >> 16) & 0xf)) & 0xff;

    /**
     * Family is an 8-bit value and is defined as: Family[7:0] = 
     * ( { 0000b, BaseFamily[3:0]} + ExtendedFamily[7:0] ).
     */
    p_cpu_feat_id->family = (((eax >> 8) & 0xf) + ((eax >> 20) & 0xff)) & 0xff;

}


void cpu_feature_detect()
{
    uint64_t value;
    uint32_t eax, ebx, ecx, edx;

    get_cpu_info(&cpu_feat_id);

    // Miscellaneous Feature Identifiers
    cpuid(CPUID_LEFT_FEATURE_IDENTIFIERS,
            &eax, &ebx, &ecx, &edx);
    support_avx = (boolean)((ecx >> 28) & 1); 
    support_osxsave = (boolean)((ecx >> 27) & 1);
    support_xsave = (boolean)((ecx >> 26) & 1);
    support_aes = (boolean)((ecx >> 25) & 1);
    support_sse42 = (boolean)((ecx >> 20) & 1);
    support_sse41 = (boolean)((ecx >> 19) & 1);
    support_cmpxchg16b = (boolean)((ecx >> 13) & 1);
    support_fma = (boolean)((ecx >> 12) & 1);
    support_ssse3 = (boolean)((ecx >> 9) & 1);
    support_sse3 = (boolean)((ecx >> 0) & 1);
    support_rdrand = (boolean)((ecx >> 30) & 1);
    support_mtrr = (boolean)((edx >> 12) & 1);

    // Structured Extended Feature Flags Enumeration
    cpuid_ex(CPUID_LEFT_STRUCTURED_EX_FEATURE_FLAGS, 0,
            &eax, &ebx, &ecx, &edx);
    support_avx2 = (boolean)((ebx >> 5) & 1);

}

boolean cpu_feature_support(X86_FEATURE feature)
{
    switch (feature) {
        case X86_FEATURE_AVX:
            return support_avx;

        case X86_FEATURE_OSXSAVE:
            return support_osxsave;

        case X86_FEATURE_XSAVE:
            return support_xsave;

        case X86_FEATURE_AES:
            return support_aes;

        case X86_FEATURE_SSE42:
            return support_sse42;

        case X86_FEATURE_SSE41:
            return support_sse41;

        case X86_FEATURE_CMPXCHG16B:
            return support_cmpxchg16b;

        case X86_FEATURE_FMA:
            return support_fma;

        case X86_FEATURE_SSSE3:
            return support_ssse3;

        case X86_FEATURE_SSE3:
            return support_sse3;

        case X86_FEATURE_RDRAND:
            return support_rdrand;

        case X86_FEATURE_MTRR:
            return support_mtrr;

        default:
            return false;
    }
}
