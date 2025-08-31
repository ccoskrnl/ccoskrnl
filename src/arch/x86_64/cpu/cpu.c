#include "cpu.h"
#include "cpu_features.h"

void __set_idtr(struct _pseudo_desc *idtr) {
    asm volatile ("lidt (%0)" : : "r"(idtr));
}

void __read_idtr(struct _pseudo_desc *idtr) {
    asm volatile ("sidt (%0)" : : "r"(idtr));
}

void __set_gdtr(struct _pseudo_desc *gdtr) {
    asm volatile ("lgdt (%0)" : : "r"(gdtr));
}

void __read_gdtr(struct _pseudo_desc *gdtr) {
    asm volatile ("sgdt (%0)" : : "r"(gdtr));
}

void __set_ldtr(uint16_t selector) {
    asm volatile ("lldt %0" : : "r"(selector));
}

uint16_t __read_ldtr() {
    uint16_t selector;
    asm volatile ("sldt %0" : "=r"(selector));
    return selector;
}

void __load_tr(uint16_t selector) {
    asm volatile ("ltr %0" : : "r" (selector));
}


/**
 * This function reads the contents of a 64-bit MSR specified by the msr parameter.
 * 
 * @param[in]       msr             Address of Model Specific Register.
 * 
 * @retval                          Value of specific msr.
 */
uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    __asm__ volatile (
            "rdmsr"
            : "=a"(low), "=d"(high)
            : "c"(msr)
            );
    return ((uint64_t)high << 32) | low;
}


/**
 * This function writes a 64-bit value to a MSR specified by the msr parameter.
 * 
 * @param[in]       msr             Address of Model Specific Register.
 * @param[in]       value           The value will be written to specific msr.
 * 
 * @retval          none.
 * 
 */
void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t low = (uint32_t)value;
    uint32_t high = (uint32_t)(value >> 32);
    __asm__ volatile (
            "wrmsr"
            :
            : "c"(msr), "a"(low), "d"(high)
            );
}


/**
 * This function calls cpuid instruction with leaf and returns processor identification and
 * feature information.
 * 
 * @param[in]       leaf            MOV eax with leaf value.
 * @param[in out]   eax             A pointer that points a uint32_t value, the eax will
 *                                  be copied into the memory field.
 * @param[in out]   ebx             A pointer that points a uint32_t value, the ebx will
 *                                  be copied into the memory field.
 * @param[in out]   ecx             A pointer that points a uint32_t value, the ecx will
 *                                  be copied into the memory field.
 * @param[in out]   edx             A pointer that points a uint32_t value, the edx will
 *                                  be copied into the memory field.
 * 
 * @retval                          CPUID returns processor identification and feature 
 *                                  information in the EAX, EBX, ECX, and EDX registers.
 *                                  The instruction’s output is dependent on the contents 
 *                                  of the leaf value upon execution.
 */
void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ volatile (
            "cpuid"
            : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
            : "a"(leaf)
            );
}


/**
 * This function calls cpuid instruction with leaf and returns processor identification and
 * feature information.
 * 
 * @param[in]       leaf            MOV eax with leaf value.
 * @param[in]       fn              To determine which function set will be called.
 * @param[in out]   eax             A pointer that points a uint32_t value, the eax will
 *                                  be copied into the memory field.
 * @param[in out]   ebx             A pointer that points a uint32_t value, the ebx will
 *                                  be copied into the memory field.
 * @param[in out]   ecx             A pointer that points a uint32_t value, the ecx will
 *                                  be copied into the memory field.
 * @param[in out]   edx             A pointer that points a uint32_t value, the edx will
 *                                  be copied into the memory field.
 * 
 * @retval                          CPUID returns processor identification and feature 
 *                                  information in the EAX, EBX, ECX, and EDX registers.
 *                                  The instruction’s output is dependent on the contents 
 *                                  of the leaf value upon execution.
 */
void cpuid_ex(uint32_t leaf, uint32_t fn, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ volatile (
            "cpuid"
            : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
            : "a"(leaf), "c"(fn)
            );
}

uint32_t _cpuid_get_apic_id()
{
    uint32_t eax, ebx, ecx, edx;
    cpuid(CPUID_LEFT_FEATURE_IDENTIFIERS, &eax, &ebx, &ecx, &edx);
    return ebx >> 24;
}



/**
 * Get the CPU cycle count. Read Time-Stamp Counter
 * 
 * @retval                          A uint64_t value which is Time-Stamp Counter.
 */
uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

uint64_t read_xcr0() {
    uint32_t eax, edx;
    __asm__ volatile (
            "xgetbv"
            : "=a" (eax), "=d" (edx)
            : "c" (0)
            );
    return ((uint64_t)edx << 32) | eax;
}

void write_xcr0(uint64_t value) {
    uint32_t eax = (uint32_t)value;
    uint32_t edx = (uint32_t)(value >> 32);
    __asm__ volatile (
            "xsetbv"
            :
            : "a" (eax), "d" (edx), "c" (0)
            );
}
