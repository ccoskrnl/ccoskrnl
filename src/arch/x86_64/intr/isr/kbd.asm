[bits 64]
extern keyboard_isr

section .text
global keyboard_isr_wrapper

keyboard_isr_wrapper:
    cli                     ; Clear interrupts
    push rax                ; Save registers
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp

    call keyboard_isr       ; Call the C function

    pop rbp                 ; Restore registers
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    sti                     ; Set interrupts
    iretq                   ; Return from interrupt