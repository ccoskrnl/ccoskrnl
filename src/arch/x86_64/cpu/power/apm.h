#ifndef __X86_CPU_APM_H__
#define __X86_CPU_APM_H__

#include "../../../../include/types.h"

typedef struct _cpu_apm_def {

    uint32_t ts : 1;                            // Temperature sensor.
    uint32_t fid : 1;                           // Frequency ID control. Function replaced by HwPstate
    uint32_t vid : 1;                           // Voltage ID control. Function replaced by HwPstate.
    uint32_t ttp : 1;                           // THERMTRIP.
    uint32_t tm : 1;                            // hardware thermal control (HTC).
    uint32_t reserved0 : 1;                     // 
    uint32_t _100MHzSteps : 1;                  // 100 MHz multiplier Control.


    /*
     * hardware P-state control. MSRC001_0061 [P-state Current Limit], MSRC001_0062 
     * [P-state Control] and MSRC001_0063 [P-state Status] exist. 
     */
    uint32_t HwPstate : 1;

    /*
     * TSC invariant. The TSC rate is ensured to be invariant across all P-States, C-States, 
     * and stop grant transitions (such as STPCLK Throttling); therefore the TSC is suitable for use as a 
     * source of time. 0 = No such guarantee is made and software should avoid attempting to use the TSC 
     * as a source of time.
     */
    uint32_t TscInvariant : 1;

    uint32_t cpb : 1;                           // core performance boost

    /*
     *  read-only effective frequency interface. 1=Indicates presence of MSRC000_00E7 
     * [Read-Only Max Performance Frequency Clock Count (MPerfReadOnly)] and MSRC000_00E8 
     * [Read-Only Actual Performance Frequency Clock Count (APerfReadOnly)].
     */
    uint32_t EffFreqRO : 1;

} cpu_apm_def_t;

#endif
