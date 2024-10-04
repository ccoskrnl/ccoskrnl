[org 0x1000]

; ┌───────────────────┐◄────── entry_addr         
; │                   │                           
; │                   │                           
; │                   │                           
; │                   │                           
; │                   │                           
; │                   │                           
; │   Startup Routine │                           
; │        Code       │                           
; │                   │                           
; │                   │                           
; │                   │                           
; │                   │                           
; │                   │                           
; │                   │                           
; ├───────────────────┤◄────── entry_addr + 0x800 
; │    MTRRs state    │                           
; ├───────────────────┤◄────── entry_addr + 0x900 
; │   Long-Mode GDT   │                           
; ├───────────────────┤◄────── entry_addr + 0xA00 
; │                   │                           
; │                   │                           
; │                   │                           
; │                   │                           
; │       Shared      │                           
; │        Data       │                           
; │        Area       │                           
; │                   │                           
; │                   │                           
; │                   │                           
; │                   │                           
; ├───────────────────┤◄────── entry_addr + 0x1000
; │                   │                           
; │     Temporary     │                           
; │     Page-Table    │                           
; │                   │                           
; └───────────────────┘                           

; Actually, we can't assume the UEFI will allocate 8Kib memory at 0x1000. We need to fix the
; entrypoint and the references of pointer if necessary(Exactly, we need in most case).

lock_var_offset equ 0x980
temporary_stack_top_offset equ 0x800
long_mode_gdt_offset equ 0x900
data_section_offset equ 0xA00

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

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Signature
    nop
    nop
    nop
    nop

    ; Acquire spinlock to ensure the startup routine code is executed by only
    ; one processor.

    ; The ebx holds the base address of ap_startup routine. we might need to
    ; fix the value before execute the following initialization.
    mov ebx, 0x1000

    ; Locates the lock variable.
    add ebx, lock_var_offset
.acquire_lock:
    mov eax, 1
    xchg eax, [ebx]
    test eax, eax
    jnz .acquire_lock


    ; Sets temporary stack pointer.
    sub ebx, lock_var_offset
    mov esp, ebx
    add esp, temporary_stack_top_offset

    ; Load GDTR
    mov eax, 0x99000
    db 0x66
    lgdt[eax]
    
    ; Load IDTR
    mov eax, 0x99000
    db 0x66
    lidt[eax]

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

    ; We need to fix the target code address for the jump instruction
    ; (Jump far, absolute, address given in operand).

    ; Clear prefetch queue
    jmp dword 0x08:protect_mode_entry

[bits 32]
protect_mode_entry:    

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
    mov eax, ebx
    add eax, long_mode_gdt_offset
    lgdt[eax]

    ; Set temporary cr3
    mov edx, ebx
    add edx, 0x1000
    mov eax, cr3
    and eax, 0xfff
    or eax, edx

    mov cr3, eax

    ; Enable IA-32e Mode
    mov ecx, 0xc0000080
    rdmsr
    or eax, 0x0101
    wrmsr

    ; Enable Paging
    mov eax, cr0
    bts eax, 31
    mov cr0, eax

    ; We need to fix the target code address for the jump instruction
    ; (Jump far, absolute, address given in operand).
    jmp 0x8:long_mode_entry

[bits 64]
long_mode_entry:

    ; Set up segment registers
    mov ax, 0x10  ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rdx, rbx
    add rdx, data_section_offset

    ; Update the number of running cores.
    lock inc qword[rdx+0x8]

    ; Load Bsp'cr3 into ap.
    mov rax, qword[rdx]
    mov cr3, rax

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
    dw 0, 0, 0