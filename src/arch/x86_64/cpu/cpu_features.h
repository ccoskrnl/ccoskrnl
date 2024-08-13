#ifndef __X86_64_CPU_FEATURES_H__
#define __X86_64_CPU_FEATURES_H__

#include "../../../include/types.h"

extern char cpu_vendor_id[16];

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