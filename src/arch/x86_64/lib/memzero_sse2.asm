[bits 64]
section .text
global memzero_sse2

; Function: zero memory using AVX2
memzero_sse2:

    ; size / ( 8 * 2)
    shr rsi, 4
    mov rcx, rsi

.memzero_sse2_copy_loop:
    
    ; VXORPD AVX CPUID Fn0000_0001_ECX[AVX] (bit 28)
    xorpd xmm0, xmm0
    ; VMOVDQU AVX CPUID Fn0000_0001_ECX[AVX] (bit 28)
    movdqu [rdi], xmm0
    add rdi, 16

    loop .memzero_sse2_copy_loop

    ret
