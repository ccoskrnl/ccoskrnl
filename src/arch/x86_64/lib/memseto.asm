[bits 64]
section .text
    global memseto

memseto:

    mov rcx, rdx
    mov rax, rdi

.__memseto_copy_loop:

    movdqu xmm0, [rsi]
    movdqu [rdi], xmm0
    add rdi, 16

    loop ._memseto_copy_loop
    
    ret 
