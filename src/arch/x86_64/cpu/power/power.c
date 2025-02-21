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
    putsds(output_bsp, "Current Pstate Limit: ", AMD_MSR_P_STATE_CUR_LIMIT_CurPstateLimit(cur_limit), "\n");
    putsds(output_bsp, "Pstate Max Value: ", AMD_MSR_P_STATE_CUR_LIMIT_PstateMaxVal(cur_limit), "\n");


    for (int i = 0; i < AMD_MSR_P_STATE_NUM; i++) {

        p_state_msr_value = rdmsr(p_state_def_msr + i);

        if (pstate->PstateEn) {
            putsds(output_bsp, "P", i, ":\n");
            putsds(output_bsp, "\tCore Frequency ID: ", pstate->CpuFid, "\n");
            putsds(output_bsp, "\tCore Divisor ID: ", pstate->CpuDid, "\n");
            putsds(output_bsp, "\tCore Voltage ID: ", pstate->CpuVid, "\n");
            putsds(output_bsp, "\tNorthbridge P-state: ", pstate->NbPstate, "\n");
            putsds(output_bsp, "\tIddValue: ", pstate->IddValue, "\n");
            putsds(output_bsp, "\tIddDiv: ", pstate->IddDiv, "\n");
        }
    }
        
    
    uint64_t cur_pstate = rdmsr(AMD_MSR_P_STATE_STATUS);
    putsds(output_bsp, "Current P-State: ", 
        AMD_MSR_P_STATE_CurPstate(cur_pstate), "\n");

}