#include "../../../../include/types.h"
#include "p_state.h"
#include "../cpu_features.h"

void transition_to_p0()
{
    // uint64_t p_state_msr_addr = AMD_MSR_P_STATE(0);
    // uint64_t p_state_msr_value = 0;
    // amd_p_state_t *pstate = (amd_p_state_t*)&p_state_msr_value;

    // for (; p_state_msr_addr < (AMD_MSR_P_STATE(0) + AMD_MSR_P_STATE_NUM); p_state_msr_addr++) {
    //     p_state_msr_value = rdmsr(p_state_msr_addr);
    // }

    // uint64_t cur_p_state_msr = AMD_MSR_P_STATE_STATUS;
    // uint64_t cur_pstate = rdmsr(cur_p_state_msr);
    // uint64_t ps = AMD_MSR_P_STATE_CurPstate(cur_pstate);
}