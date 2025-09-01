#include "../../../../include/types.h"
#include "p_state.h"
#include "../cpu_features.h"

#include "../../../../include/go/go.h"

void transition_to_p0()
{

    uint64_t p_state_def_msr = AMD_MSR_P_STATE(0);
    uint64_t p_state_msr_value = 0;
    amd_p_state_t *pstate = (amd_p_state_t*)&p_state_msr_value;

    uint64_t cur_limit = rdmsr(AMD_MSR_P_STATE_CUR_LIMIT);
    putsds(bsp_window, "Current Pstate Limit: ", AMD_MSR_P_STATE_CUR_LIMIT_CurPstateLimit(cur_limit), "\n");
    putsds(bsp_window, "Pstate Max Value: ", AMD_MSR_P_STATE_CUR_LIMIT_PstateMaxVal(cur_limit), "\n");


    for (int i = 0; i < AMD_MSR_P_STATE_NUM; i++) {

        p_state_msr_value = rdmsr(p_state_def_msr + i);

        if (pstate->PstateEn) {
            putsds(bsp_window, "P", i, ":\n");
            putsds(bsp_window, "\tCore Frequency ID: ", pstate->CpuFid, "\n");
            putsds(bsp_window, "\tCore Divisor ID: ", pstate->CpuDid, "\n");
            putsds(bsp_window, "\tCore Voltage ID: ", pstate->CpuVid, "\n");
            putsds(bsp_window, "\tNorthbridge P-state: ", pstate->NbPstate, "\n");
            putsds(bsp_window, "\tIddValue: ", pstate->IddValue, "\n");
            putsds(bsp_window, "\tIddDiv: ", pstate->IddDiv, "\n");
        }
    }


    uint64_t cur_pstate = rdmsr(AMD_MSR_P_STATE_STATUS);
    putsds(bsp_window, "Current P-State: ", 
            AMD_MSR_P_STATE_CurPstate(cur_pstate), "\n");

}
