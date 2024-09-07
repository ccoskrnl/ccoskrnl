[bits 64]
section .text
global memcpy_avx

; Function: memcpy using AVX2
memcpy_avx:
    
    ; size / (8 * 4)
    shr rdx, 5
    mov rcx, rdx
    mov rax, rdi

.memcpy_avx_copy_loop:

    vmovdqu ymm0, [rsi]
    vmovdqu [rdi], ymm0
    add rsi, 32
    add rdi, 32
    loop .memcpy_avx_copy_loop

    ret
