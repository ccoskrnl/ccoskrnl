[bits 64]
section .text
global memzero_avx

; Function: zero memory using AVX2
memzero_avx:

    ; size / ( 8 * 4)
    shr rsi, 5
    mov rcx, rsi

.memzero_avx_copy_loop:
    
    ; VXORPD AVX CPUID Fn0000_0001_ECX[AVX] (bit 28)
    vxorpd ymm0, ymm0
    ; VMOVDQU AVX CPUID Fn0000_0001_ECX[AVX] (bit 28)
    vmovdqu [rdi], ymm0
    add rdi, 32

    loop .memzero_avx_copy_loop

    ret
