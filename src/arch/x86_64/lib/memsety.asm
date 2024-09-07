[bits 64]
section .text
    global memsety

memsety:

    mov rcx, rdx
    mov rax, rdi

.__memsety_copy_loop:

    ymovdqu ymm0, [rsi]
    ymovdqu [rdi], ymm0
    add rdi, 32

    loop ._memsety_copy_loop
    
    ret 
