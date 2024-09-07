[bits 64]
extern _intr_pm_handler

; In IA-32e mode, the RSP is aligned to a 16-byte boundary before pushing the stack frame. The stack 
; frame itself is aligned on a 16-byte boundary when the interrupt handler is called. The processor can arbitrarily 
; realign the new RSP on interrupts because the previous (possibly unaligned) RSP is unconditionally saved on the 
; newly aligned stack. The previous RSP will be automatically restored by a subsequent IRET.

; LeetSpeak 1337: 
; CC05 converted from ccos
MagicNumber equ 0x43433035

section .text

; Define a macro, called INTERRUPT_HANDLER, which takes two parameters.
%macro INTERRUPT_HANDLER 2

; Define a tag, a macro, takes a parameter. And %1 is the parameter.
; It's used as a part of the macro.
._intr_handler_%1:
; If the second parameter doesn't exist, we need to push a magicnumber.
%ifn %2
    push MagicNumber
%endif
; Then, we push vector number and jump to intr_handler_entry.
    push %1
    jmp ._intr_handler_entry

%endmacro





    
; 1st: error code with vector number
; 2nd: RIP
; 3rd: CS
; 4th: RFLAGS
; 5th: RSP
; 6th: SS
._intr_handler_entry:
    cli
    ; error code with vector number
    pop rax
    pop rdi
    shl rax, 32
    or rdi, rax

    mov rsi, qword [rsp]
    mov rdx, qword [rsp + 0x8]
    mov rcx, qword [rsp+0x10]
    mov r8, qword [rsp+0x18]
    mov r9, qword [rsp+0x20]

    ; call Protected-Mode Exceptions and Interrupts Handler.
    mov rax, _intr_pm_handler
    call rax
    ; iret
    hlt


    



INTERRUPT_HANDLER 0x00, 0; divide by zero
INTERRUPT_HANDLER 0x01, 0; debug
INTERRUPT_HANDLER 0x02, 0; non maskable interrupt
INTERRUPT_HANDLER 0x03, 0; breakpoint

INTERRUPT_HANDLER 0x04, 0; overflow
INTERRUPT_HANDLER 0x05, 0; bound range exceeded
INTERRUPT_HANDLER 0x06, 0; invalid opcode
INTERRUPT_HANDLER 0x07, 0; device not avilable

INTERRUPT_HANDLER 0x08, 1; double fault
INTERRUPT_HANDLER 0x09, 0; coprocessor segment overrun
INTERRUPT_HANDLER 0x0a, 1; invalid TSS
INTERRUPT_HANDLER 0x0b, 1; segment not present

INTERRUPT_HANDLER 0x0c, 1; stack segment fault
INTERRUPT_HANDLER 0x0d, 1; general protection fault
INTERRUPT_HANDLER 0x0e, 1; page fault
INTERRUPT_HANDLER 0x0f, 0; reserved

INTERRUPT_HANDLER 0x10, 0; x87 floating point exception
INTERRUPT_HANDLER 0x11, 1; alignment check
INTERRUPT_HANDLER 0x12, 0; machine check
INTERRUPT_HANDLER 0x13, 0; SIMD Floating - Point Exception

INTERRUPT_HANDLER 0x14, 0; Virtualization Exception
INTERRUPT_HANDLER 0x15, 1; Control Protection Exception
INTERRUPT_HANDLER 0x16, 0; reserved
INTERRUPT_HANDLER 0x17, 0; reserved

INTERRUPT_HANDLER 0x18, 0; reserved
INTERRUPT_HANDLER 0x19, 0; reserved
INTERRUPT_HANDLER 0x1a, 0; reserved
INTERRUPT_HANDLER 0x1b, 0; reserved

INTERRUPT_HANDLER 0x1c, 0; reserved
INTERRUPT_HANDLER 0x1d, 0; reserved
INTERRUPT_HANDLER 0x1e, 0; reserved
INTERRUPT_HANDLER 0x1f, 0; reserved



section .data
global _intr_handler_entry_table

; handler_entry_table defines all addresses of pm handler.
; each member is a function address.
; _intr_handler_entry_0x[xx] be treated as 'tag' in assembly.
_intr_handler_entry_table:
    dq ._intr_handler_0x00
    dq ._intr_handler_0x01
    dq ._intr_handler_0x02
    dq ._intr_handler_0x03
    dq ._intr_handler_0x04
    dq ._intr_handler_0x05
    dq ._intr_handler_0x06
    dq ._intr_handler_0x07
    dq ._intr_handler_0x08
    dq ._intr_handler_0x09
    dq ._intr_handler_0x0a
    dq ._intr_handler_0x0b
    dq ._intr_handler_0x0c
    dq ._intr_handler_0x0d
    dq ._intr_handler_0x0e
    dq ._intr_handler_0x0f
    dq ._intr_handler_0x10
    dq ._intr_handler_0x11
    dq ._intr_handler_0x12
    dq ._intr_handler_0x13
    dq ._intr_handler_0x14
    dq ._intr_handler_0x15
    dq ._intr_handler_0x16
    dq ._intr_handler_0x17
    dq ._intr_handler_0x18
    dq ._intr_handler_0x19
    dq ._intr_handler_0x1a
    dq ._intr_handler_0x1b
    dq ._intr_handler_0x1c
    dq ._intr_handler_0x1d
    dq ._intr_handler_0x1e
    dq ._intr_handler_0x1f
