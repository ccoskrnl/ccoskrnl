[org 0x1000]

; Waits on the BIOS initialization Lock Semaphore. When control of the semaphore is attained, initialization 
; continues

; Loads the microcode update into the processor.

; Initializes the MTRRs (using the same mapping that was used for the BSP).

; Enables the cache

; Switches to protected mode

[bits 16]
ap_entry_point:

    cli
    wbinvd

    ; mov ax, cs
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax


    ; Sets temporary stack pointer.
    mov sp, 0x1800
    mov bx, 0x1800

    ; Loads IDTR and GDTR
    lgdt[gdtr_32]
    lidt[idtr_32]

;   BIOS initialization routine should maintain the conherency of memory region type. 
;   Otherwise, we need to save the mtrrs of bsp and load into ap's mtrrs. In here, 
;   we assume that the BIOS done. 

;   The following code to maintain the conherency of different processor.

;     ; Enter the no-fill cache mode. 
;     ; (Set the CD flag in control register CR0 to 1 and the NW flag to 0.)
;     mov eax, cr0
;     or eax, 0x40000000
;     and eax, 0xDFFFFFFF
;     mov cr0, eax

;     ; Read MTRR CAP MSR.
;     mov ecx, 0xfe
;     rdmsr

;     ; Get Vcnt and save into si.
;     mov si, ax
;     and si, 0xff

;     bt ax, 8
;     jnc .map_phys

;     ; Write IA32_MTRR_FIX64K_00000 MSR
;     mov cx, 0x250

;     mov eax, dword [bx + 0]
;     mov edx, dword [bx + 4]
;     wrmsr
;     add bx, 0x8

;     ; Write IA32_MTRR_FIX16K_xxxxx MSRs
;     mov cx, 0x258
;     mov eax, dword [bx + 0]
;     mov edx, dword [bx + 4]
;     wrmsr
;     add bx, 0x8

;     inc cx
;     mov eax, dword [bx + 0]
;     mov edx, dword [bx + 4]
;     wrmsr
;     add bx, 0x8

;     ; Write IA32_MTRR_FIX4K_xxxxx MSRs
;     mov cx, 8
;     mov di, 0x268
; .write_to_fix4x_msrs:

;     ; Exchange the values of cx and di
;     mov ax, cx
;     mov cx, di
;     mov di, ax

;     mov eax, dword [bx + 0]
;     mov edx, dword [bx + 4]
;     wrmsr
;     add bx, 0x8

;     inc cx

;     ; Exchange the values of cx and di
;     mov ax, cx
;     mov cx, di
;     mov di, ax

;     loop .write_to_fix4x_msrs

; .map_phys:

;     mov cx, si
;     mov di, 0x200
; .write_to_phys:

;     ; Exchange the values of cx and di
;     mov ax, cx
;     mov cx, di
;     mov di, ax

;     mov eax, dword [bx + 0]
;     mov edx, dword [bx + 4]
;     wrmsr
;     add bx, 0x8

;     inc cx

;     ; Exchange the values of cx and di
;     mov ax, cx
;     mov cx, di
;     mov di, ax

;     loop .write_to_phys

;     ; Enter the no-fill cache mode. 
;     ; (Set the CD flag in control register CR0 to 1 and the NW flag to 0.)
;     mov eax, cr0
;     and eax, 0x9FFFFFFF
;     mov cr0, eax

;     mov cx, 0x2ff
;     rdmsr

    ; Enter Protected-Mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Clear prefetch queue
    ; TODO CPU received signal SIGTRAP, Trace/breakpoint trap.
    ; 0x000000000000fff0 in ?? ()
    jmp dword 0x08:protect_mode_code

[bits 32]
protect_mode_code:    

    mov ax, 0x10
    mov ds, ax
    mov es, ax


    mov cr4, eax
    ; Enable physical-address extensions(PAE)
    or eax, 0x20
    ; Use 4-level Paging.
    and eax, 0xFFFFEFFF
    mov cr4, eax

    ; Load Long-Mode GDT
    mov ebx, 0x1900
    lgdt[ebx]

    ; Get CR3, which is stored at 0x1A00.
    mov ebx, 0x1A00
    mov edi, dword[ebx]
    mov eax, edi

    ; Set CR3
    mov cr3, eax

    ; Enable IA-32e Mode
    mov ecx, 0xc0000080
    rdmsr
    or eax, 0x0101
    wrmsr

    ; enable Paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    
    jmp long_mode

[bits 64]
long_mode:

    ; Set up segment registers
    mov ax, 0x10  ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax


    int3


    mov cr3, rdi

    ; is_ap
    mov rsi, 1

    ; Far return to reload CS
    push 0x8  ; Code segment selector
    mov rax, 0xfffff00000001000
    push rax
    ; goto kernel
    retfq





section .data
    align 16
gdtr_32:
    dw (gdt_end_32 - gdt_base_32) - 1
    dq gdt_base_32
    dw 0, 0, 0

gdt_base_32:
    dq 0                ; NULL seg.
    ; Temporary kernel code segment
    dq 0x00cf9a000000ffff
    ; Temporary kernel data segment
    dq 0x00cf92000000ffff
gdt_end_32:
    dq 0


idtr_32:
    dw 0
    dq 0
    ; dw (idt_end_32 - idt_base_32) - 1
    ; dq idt_base_32
    dw 0, 0, 0
        
idt_base_32:
    dq 0, 0, 0, 0, 0, 0, 0, 0
    dq 0, 0, 0, 0, 0, 0, 0, 0
    dq 0, 0, 0, 0, 0, 0, 0, 0
    dq 0, 0, 0, 0, 0, 0, 0, 0
idt_end_32:
