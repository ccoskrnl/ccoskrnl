[bits 64]

extern _intr_gpf_handler

section .text
    global _intr_gpf_handler_entry
    global _intr_intr_handler_test


; In IA-32e mode, the RSP is aligned to a 16-byte boundary before pushing the stack frame. The stack 
; frame itself is aligned on a 16-byte boundary when the interrupt handler is called. The processor can arbitrarily 
; realign the new RSP on interrupts because the previous (possibly unaligned) RSP is unconditionally saved on the 
; newly aligned stack. The previous RSP will be automatically restored by a subsequent IRET.
    
; 1st: error
; 2nd: RIP
; 3rd: CS
; 4th: RFLAGS
; 5th: RSP
; 6th: SS
_intr_gpf_handler_entry:

    xor rcx, rcx

    pop rdi
    shl rdi, 32
    or rdi, 13
    mov rsi, qword [rsp]
    mov rdx, qword [rsp+0x8]
    mov rcx, qword [rsp+0x10]
    mov r8, qword [rsp+0x18]
    mov r9, qword [rsp+0x20]
    
    mov rax, _intr_gpf_handler
    call rax
    iret

    ud2


_intr_intr_handler_test:

    nop
    nop
    nop
    nop
    nop
    nop
    nop
