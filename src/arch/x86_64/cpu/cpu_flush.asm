[bits 64]
section .text
global _cpu_reload_seg_regs

; Function: reload_seg_regs - Reload segment registers
; Parameters:
;   di - Code segment selector
;   si - Data segment selector
_cpu_reload_seg_regs:
    mov ax, si
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss ,ax

    xor rax, rax
    mov rax, rdi
    push rax
    lea rax, [rel next]
    push rax
    retfq
next:
    ret