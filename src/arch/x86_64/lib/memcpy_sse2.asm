[bits 64]
section .text
global memcpy_sse2

; Function: memcpy using SSE2
memcpy_sse2:
    
    ; size / (8 * 2)
    shr rdx, 4
    mov rcx, rdx
    mov rax, rdi

memcpy_sse2_copy_loop:

    movdqu xmm0, [rsi]
    movdqu [rdi], xmm0
    add rsi, 16
    add rdi, 16
    loop memcpy_sse2_copy_loop

    ret