/*
Refs: https://www.amd.com/content/dam/amd/en/documents/archived-tech-docs/programmer-references/55072_AMD_Family_15h_Models_70h-7Fh_BKDG.pdf
p654-657
MSRC001_0061 P-state Current Limit
MSRC001_0062 P-state Control
MSRC001_0063 P-state Status
MSRC001_00[6B:64] P-state [7:0]
MSRC001_0070 COFVID Control

*/


#ifndef __X86_CPU_AMD_P_STATE_H__
#define __X86_CPU_AMD_P_STATE_H__

#include "../../../../include/types.h"



/**
 * MSRC001_0061 P-state Current Limit 
 * Read; GP-write; Per-compute-unit; Updated-by-hardware.
 */
#define AMD_MSR_P_STATE_CUR_LIMIT                                       0xC0010061
/**
 * CurPstateLimit: current P-state limit. Specifies the highest-performance non-boosted P-state
 * (lowest value) allowed. CurPstateLimit is always bounded by MSRC001_0061[PstateMaxVal].
 * Attempts to change the CurPstateLimit to a value greater (lower performance) than 
 * MSRC001_0061[PstateMaxVal] leaves CurPstateLimit unchanged. This field uses software P-state 
 * numbering.
 */
#define AMD_MSR_P_STATE_CUR_LIMIT_CurPstateLimit(msr_value)             ((uint64_t)msr_value & (uint64_t)0x7)

/**
 * PstateMaxVal: P-state maximum value. Specifies the lowest-performance non-boosted P-state 
 * (highest non-boosted value) allowed. Attempts to change MSRC001_0062[PstateCmd] to a lower-
 * performance P-state (higher value) are clipped to the value of this field. This field uses software 
 * P-state numbering.
 */
#define AMD_MSR_P_STATE_CUR_LIMIT_PstateMaxVal(msr_value)               (((uint64_t)msr_value & (uint64_t)0x70) >> 4)




/* MSRC001_0062 P-state Contro */
#define AMD_MSR_P_STATE_CONTROL                                         0xC0010062
/**
 * PstateCmd: P-state change command. Read-write; Not-same-for-all. Cold reset value varies by
 * product; after a warm reset, value initializes to the P-state the core was in prior to the reset. Writes to
 * this field cause the core to change to the indicated non-boosted P-state number, specified by
 * MSRC001_00[6B:64]. 0=P0, 1=P1, etc. P-state limits are applied to any P-state requests made 
 * through this register. Reads from this field return the last written value, regardless of whether any
 * limit are applied. This field uses software P-state numbering.
 */
#define AMD_MSR_P_STATE_CONTROL_PstateCmd(p_state_cmd)                  ((uint64_t)((uint64_t)(p_state_cmd) & 0x7))


/**
 * MSRC001_0063 P-state Status 
 * Read; GP-write; Per-compute-unit; Updated-by-hardware.
 */
#define AMD_MSR_P_STATE_STATUS                                          0xC0010063
/**
 * CurPstate: current P-state. Cold reset: Varies by product. This field provides the frequency
 * component of the current non-boosted P-state of the core (regardless of the source of the P-state change,
 * including MSRC001_0062[PstateCmd]. 0=P0, 1=P1, etc. The value of this field is updated when the
 * COF transitions to a new value associated with a P-state. This field uses software P-state numbering.
 */
#define AMD_MSR_P_STATE_CurPstate(msr_value)                            ((uint64_t)(msr_value) & (uint64_t)0x7)

/**
 * Register     Function
 * MSRC001_0064 P-state 0
 * MSRC001_0065 P-state 1
 * MSRC001_0066 P-state 2
 * MSRC001_0067 P-state 3
 * MSRC001_0068 P-state 4
 * MSRC001_0069 P-state 5
 * MSRC001_006A P-state 6
 * MSRC001_006B P-state 7
 */

#define AMD_MSR_P_STATE(index)                                          (0xC0010064 + index)
#define AMD_MSR_P_STATE_NUM                                             8

typedef struct _amd_p_state
{
    /**
     * CpuFid[5:0]: core frequency ID. Read-write. Except as required by [Processor-Systemboard
     * Current Delivery Compatibility Check], software should not modify this field. Specifies the 
     * core frequency multiplier. The core COF is a function of CpuFid and CpuDid, and defined by Core COF.
     */
    uint64_t CpuFid : 6;

    /**
     * cpuDid: core divisor ID. Read-write. Except as required by [Processor-Systemboard 
     * Current Delivery Compatibility Check], software should not modify this field. Specifies the 
     * frequency divisor; see CpuFid. 
     *     Bits Description
     *     0h Divide by 1
     *     1h Divide by 2
     *     3h Divide by 4
     *     3h Divide by 8
     *     4h Divide by 16
     *     7h-5h Reserved.
     */
    uint64_t CpuDid : 3;

    /**
     * CpuVid: core VID. Read-write. Except as required by [Processor-Systemboard 
     * Current Delivery Compatibility Check], software should not modify this field.
     * The CpuVid field in these registers is required to be programmed to the same value in all cores of a processor, 
     * but are allowed to be different between processors in a multi-processor system. All other fields in these 
     * registers are required to be programmed to the same value in each core of the coherent fabric
     */
    uint64_t CpuVid : 8;

    /**
     * RAZ
     */
    uint64_t RAZ0 : 5;

    /**
     * NbPstate: Northbridge P-state.
     * 
     * IF (MSRC001_0071) THEN 
     *      Read-only.
     * ELSE
     *      Read-write.
     * ENDIF.
     * 
     * 1: Low Performance NB P-state.
     * 0: High performance NB P-state.
     * 
     * If this bit is set in any given P-state register, then it must also be
     * set in all enabled lower performance P-state registers as well.
     * Equivalent P-states in each code must program this bit to the same value.
     */
    uint64_t NbPstate : 1;

    /**
     * RAZ
     */
    uint64_t RAZ1 : 9;

    /**
     * IddValue: current value. Read-write.
     * 
     * After a reset, IddDiv and IddValue combine to specify the expected maximum
     * current dissipation of a single core that is in the P-state corresponding to
     * MSR number. These values are intended to be used to create ACPI-defined _PSS
     * objects and to perform the [Processor-Systemboard Current Delivery Compatibility
     * Check]. The values are expressed in amps; they are not intended to convey final
     * product power levels; they may not match the power levels specified in the Power
     * and Thermal Datasheets. These field are encoded as follows:
     *      IddDiv      Descipition
     *      00b         IddValue / 1 A, Range: 0 to 255 A.
     *      01b         IddValue / 10 A, Range: 0 to 25.5 A
     *      10b         IddValue / 100 A, Range: 0 to 2.55 A.
     *      11b         Reserved
     */
    uint64_t IddValue : 9;

    /**  IddDiv: current divisor. Read-write. See IddValue. */
    uint64_t IddDiv : 2;

    /**
     * RAZ
     */
    uint64_t RAZ2 : 20;

    /**
     * PstateEn. Read-write.
     * 1: The P-state specified by this MSR is valid.
     * 0: The P-state specified by this MSR is not valid.
     * The purpose of this register is to indicate if the rest of the P-state
     * information in the register is valid after a reset; it controls no hardware.
     */
    uint64_t PstateEn : 1;


} __attribute__ ((packed)) amd_p_state_t;


#endif
