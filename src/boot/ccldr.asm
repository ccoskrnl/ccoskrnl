kernel_pml4_offset equ 0x1E0
kernel_space_virtual_base_addr equ 0xFFFFF00000000000
magic_number equ 0x6964616B73

ccldr_routine_size equ 0x1000

seg_selector_code equ (1 << 3)
seg_selector_data equ (2 << 3)

; Integer registers

; System V ABI Description:
; Return value:
;   rax

; Arguments:
;   rdi
;   rsi
;   rdx
;   rcx
;   r8
;   r9

; Caller saved:
;   r10
;   r11

; Callee saved:
;   rbx
;   rbp
;   r12
;   r13
;   r14
;   r15


section .text
global bsp_init


[bits 64]
bsp_init:

    nop
    nop
    nop
    nop

    mov ax, ss
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ax, ss

    ; Zero general-purpose registers
    xor rax, rax

    xor rsi, rsi
    xor rdx, rdx
    xor rcx, rcx
    xor r8, r8
    xor r9, r9

    xor rbx, rbx
    xor rbp, rbp

    ; The r8 stores pointer that points to machine_info struct.
    mov r8, rdi

    ; Get ccldr base addr.
    mov r9, qword [r8 + 0x10]
    ; Get ccldr size.
    mov rbp, r9
    add rbp, 0x10000


    ; Set temporary kernel stack pointer.
    ; before entering bsp_init, we already have allocated MACHINE_INFO_SIZE bytes memory.
    ; rsp -> ccldr base + machine info structure size.
    ; rsp and rbp used as temporary stack pointer and pointer to the top of stack.
    mov rsp, rbp


    mov rax, magic_number
    push rax                ; push magic number, used by ccldr to verify it's loaded by the bootloader correctly.
                            ; magic number stored at [rbp - 0x8]

    ; Determine how to construct ccldr page table based on it's base address
    ; Calculate pml4_base
    add r9, 0x18000         ; pml4_base = ccldr_base + MACHINE_INFO_SIZE + 0x4000
    push r9                 ; pml4_base stored at [rbp - 0x10]

    ; Calculate ccldr number of ptes
    push 0                  ; number of ptes of ccldr at [rbp - 0x18]
    push 0                  ; number of pdes of ccldr at [rbp - 0x20]
    push 0                  ; number of pdptes of ccldr at [rbp - 0x28]
    push 0                  ; number of pml4es of ccldr at [rbp - 0x30]
    mov rdi, qword [r8 + 0x18]
    mov rsi, qword [r8 + 0x10]
    mov rdx, rsp
    call func_calculate_num_of_ptes_with_base_addr;

    ; Calculate krnl number of ptes
    push 0                  ; number of ptes of krnl at [rbp - 0x38]
    push 0                  ; number of pdes of krnl at [rbp - 0x40]
    push 0                  ; number of pdptes of krnl at [rbp - 0x48]
    push 0                  ; number of pml4es of krnl at [rbp - 0x50]
    mov rdi, qword [r8 + 0x8]
    mov rsi, rsp
    call func_calculate_num_of_ptes;

    ; Calculate ccldr_pdpt_base
    mov rdx, qword[rbp - 0x10] ; get pml4_base
    add rdx, 0x1000             ; ccldr_pdpt_base = pml4_base + 0x1000
    push rdx                ; ccldr_pdpt_base at [rbp - 0x58]

    ; Calculate krnl_pdpt_base
    mov rcx, qword [rbp - 0x30] ; get num_of_pml4es_of_ccldr
    shl rcx, 12
    add rdx, rcx            ; krnl_pdpt_base = (num_of_pml4es_of_ccldr) * 0x1000 + ccldr_pdpt_base
    push rdx                ; krnl_pdpt_base at [rbp - 0x60]

    ; Calculate ccldr_pd_base
    mov rcx, qword [rbp - 0x50] ; get num_of_pml4es_of_krnl
    shl rcx, 12
    add rdx, rcx            ; ccldr_pd_base = (num_of_pml4es_of_krnl) * 0x1000 + krnl_pdpt_base
    push rdx                ; ccldr_pd_base at [rbp - 0x68]

    ; Calculate krnl_pd_base
    mov rcx, qword [rbp - 0x28] ; get num_of_pdptes_of_ccldr
    shl rcx, 12
    add rdx, rcx            ; krnl_pd_base = (num_of_pdptes_of_ccldr) * 0x1000 + ccldr_pd_base
    push rdx                ; krnl_pd_base at [rbp - 0x70]

    ; Calculate ccldr_pt_base
    mov rcx, qword [rbp - 0x40] ; get num_of_pdptes_of_krnl
    shl rcx, 12
    add rdx, rcx            ; ccldr_pt_base = (num_of_pdptes_of_krnl) * 0x1000 + krnl_pd_base
    push rdx                ; ccldr_pt_base at [rbp - 0x78]

    ; Calculate krnl_pt_base
    mov rcx, qword [rbp - 0x20] ; get num_of_pdes_of_ccldr
    shl rcx, 12
    add rdx, rcx
    push rdx                ; krnl_pt_base at [rbp - 0x80]


    ; Set PML4

    ; For ccldr
    ; Locate offset of ccldr pdpt from beginning of pml4
    ; Get ccldr base addr
    mov rbx, qword [r8 + 0x10]
    shr rbx, 39
    and rbx, 0x1FF
    shl rbx, 3
    mov rdi, qword[rbp - 0x10]
    add rdi, rbx            ; rdi: PML4[offset_of_ccldr]

    mov rsi, qword [rbp - 0x58] ; rsi: ccldr_pdpt_base

    mov rcx, qword [rbp - 0x30]              ; rcx: number of pml4es of ccldr

    call func_fill_pte


    ; For krnl
    mov rdi, kernel_pml4_offset          ; PML4[kernel_pml4_offset * 0x8] -> krnl_pdpt_base
    shl rdi, 3
    mov rbx, qword [rbp - 0x10]
    add rdi, rbx             ; rdi: pml4_base + (kernel_pml4_offset * 0x8)

    mov rsi, qword [rbp - 0x60] ; rsi: krnl_pdpt_base

    mov rcx, qword [rbp - 0x50]              ; rcx: number of pml4e of krnl

    call func_fill_pte


    ; Set PDPT

    ; Set Ccldr PDPT
    ; Calculate offset_of_ccldr from beginning of pdpt
    mov rbx, qword [r8 + 0x10]
    shr rbx, 30
    and rbx, 0x1FF
    shl rbx, 3

    mov rdi, qword [rbp - 0x58] ; get ccldr_pdpt_base
    add rdi, rbx    ; rdi: ccldr_pdpt_base[offset_of_ccldr from beginning of pdpt]

    mov rsi, qword [rbp - 0x68]     ; rsi: ccldr_pd_base
    mov rcx, qword [rbp - 0x28]     ; rdi: number of pdptes of ccldr
    call func_fill_pte

    ; Set krnl pdpt
    mov rdi, qword [rbp - 0x60]     ; rdi: krnl_pdpt_base
    mov rsi, qword [rbp - 0x70]     ; rsi: krnl_pd_base
    mov rcx, qword [rbp - 0x48]     ; rcx: number of pdptes of krnl
    call func_fill_pte

    ; Set PD

    ; Set Ccldr PD
    ; Calculate offset_of_ccldr from beginning of pd
    mov rbx, qword [r8 + 0x10]
    shr rbx, 21
    and rbx, 0x1FF
    shl rbx, 3

    mov rdi, qword [rbp - 0x68]; get ccldr_pd_base
    add rdi, rbx    ; rdi: ccldr_pd_base[offset_of_ccldr from beginning of pd]

    mov rsi, qword [rbp - 0x78] ; get ccldr_pt_base
    mov rcx, qword [rbp - 0x20] ; get number of pdes of ccldr
    call func_fill_pte

    ; Set krnl PD
    mov rdi, qword [rbp - 0x70] ; get krnl_pd_base
    mov rsi, qword [rbp - 0x80] ; get krnl_pt_base
    mov rcx, qword [rbp - 0x40] ; get number of pdes of krnl
    call func_fill_pte


    ; Set PT

    ; Set ccldr pt
    ; Calculate offset of ccldr from beginning of pt
    mov rbx, qword [r8 + 0x10]
    shr rbx, 12
    and rbx, 0x1ff
    shl rbx, 3

    mov rdi, qword [rbp - 0x78] ; get ccldr_pt_base
    add rdi, rbx ; rdi: ccldr_pt_base[offset of ccldr from beginning of pt]

    mov rsi, qword [r8 + 0x10]  ; get ccldr_base

    mov rcx, qword [rbp - 0x18] ; get number of ptes of ccldr
    call func_fill_pte

    ; Set krnl pt
    mov rdi, qword [rbp - 0x80] ; get krnl_pt_base
    mov rsi, qword [r8]         ; get krnl_base
    mov rcx, qword [rbp - 0x38] ; get number of ptes of krnl
    call func_fill_pte


    mov rcx, cr3
    and rcx, 0xfff
    mov rax, qword[rbp - 0x10]
    or rax, rcx

    ; Backup cr3 value into rbx
    mov rbx, rax
    ; Set cr3
    mov cr3, rax


    cli
    mov rdi, r8

    ; Get ccldr_base
    mov r8, qword[rdi+0x10]
    ; Locate gdt base addr
    add r8, 0x11000
    mov r9, r8
    ; Locate gdtr::base
    add r9, 0x22
    ; Set gdtr::base
    mov qword[r9],r8
    ; Set gdtr
    sub r9, 2
    lgdt[r9]

    xor rsi, rsi

    ; Set up segment registers
    mov ax, seg_selector_data  ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov cr3, rbx

    ; Far return to reload CS
    push seg_selector_code  ; Code segment selector
    mov rax, 0xfffff00000001000
    push rax
    ; Goto krnl_start
    retfq


    hlt


; Routine Description:
;   Fill Page table entry
; Parameters:
;   rdi 1st parameter: pte address
;   rsi 2st parameter: pt_base
;   rcx 3st parameter: number of entrys
; Returned Value:
;   returned 0
func_fill_pte:

    .fill_pte:
        mov rax, rsi
        or rax, 3
        mov [rdi], rax
        add rsi, 0x1000
        add rdi, 0x8
        loop .fill_pte
    xor rax, rax
    ret




; Routine Description:

;   The routine calculates the number of pdptes, the number of
; pdes and the number of ptes with specified byte size respectively.

; Parameters:
;   rdi 1st parameter: byte size
;   rsi 2st parameter: uint64_t addr[4]
;       addr[0]: number of pml4es
;       addr[1]: number of pdptes
;       addr[2]: number of pdes
;       addr[3]: number of ptes

; Returned Value:
;   None.

func_calculate_num_of_ptes:
    push rbx
    push r8
    push rbp
    mov rbp, rsp

    mov rbx, rdi    ; backup size

    mov rax, rdi
    shr rdi, 39
    mov rdx, 0x7FFFFFFFFF
    and rax, rdx
    cmp rax, 0
    je .LPML4
    inc rdi
.LPML4:
    mov qword[rsi], rdi

    mov rdi, rbx
    mov rax, rdi
    shr rdi, 30
    and rax, 0x3fffffff
    cmp rax, 0
    je .LPDPT
    inc rdi
.LPDPT:
    mov qword[rsi+0x8], rdi

    mov rdi, rbx
    mov rax, rdi
    shr rdi, 21
    and rax, 0x1FFFFF
    cmp rax, 0
    je .LPD
    inc rdi
.LPD:
    mov qword[rsi+0x10], rdi

    mov rdi, rbx
    mov rax, rdi
    shr rdi, 12
    and rax, 0xFFF
    cmp rax, 0
    je .LPT
    inc rdi
.LPT:
    mov qword[rsi+0x18], rdi

    mov rsp, rbp
    pop rbp
    pop r8
    pop rbx
    ret





; Routine Description:

;   The routine calculates the number of pdptes, the number of
; pdes and the number of ptes with specified byte size and base addr respectively.

; Parameters:
;   rdi 1st parameter: byte size
;   rsi 2st parameter: base addr
;   rdx 3st parameter: uint64_t addr[4]
;       addr[0]: number of pml4es
;       addr[1]: number of pdptes
;       addr[2]: number of pdes
;       addr[3]: number of ptes

; Returned Value:
;   None.

func_calculate_num_of_ptes_with_base_addr:
    push rbx
    push r8
    push rbp
    mov rbp, rsp

    mov rcx, rsi    ; rcx backup base
    mov rbx, rdi    ; rbx backup size

    ; rax: remainder
    mov rax, rdi
    shr rdi, 39
    mov rsi, 0x7fffffffff
    and rax, rsi
    cmp rax, 0
    je .num_of_pml4es_calculation_completed
    inc rdi
.num_of_pml4es_calculation_completed:
    mov qword[rdx], rdi     ; store num_of_pml4es




    mov r8, rcx
    shr r8, 30
    and r8, 0x1ff

    mov rdi, rbx
    mov rax, rdi
    shr rdi, 30
    and rax, 0x3fffffff
    cmp rax, 0
    je .num_of_pdptes_calculation_completed
    inc rdi

.num_of_pdptes_calculation_completed:
    mov qword[rdx+0x8], rdi

    ; if ((offset + (*p_num_of_pdptes)) > (0x1FF * (*p_num_of_pml4es))) {
    ;     *p_num_of_pml4es += 1;
    ; }
    add r8, rdi
    mov r9, qword[rdx]
    shl r9, 9
    cmp r8, r9
    jna .done_for_num_of_pml4e
    mov rdi, qword[rdx]
    inc rdi
    mov qword[rdx], rdi
.done_for_num_of_pml4e:



    mov r8, rcx
    shr r8, 21
    and r8, 0x1ff

    mov rdi, rbx
    mov rax, rdi
    shr rdi, 21
    and rax, 0x1FFFFF
    cmp rax, 0
    je .num_of_pdes_calculation_completed
    inc rdi

.num_of_pdes_calculation_completed:
    mov qword[rdx+0x10], rdi

    add r8, rdi
    mov r9, qword[rdx + 0x8]
    shl r9, 9
    cmp r8, r9
    jna .done_for_num_of_pdptes
    mov rdi, qword[rdx + 0x8]
    inc rdi
    mov qword[rdx + 0x8], rdi
.done_for_num_of_pdptes:

    mov r8, rcx
    shr r8, 12
    and r8, 0x1ff

    mov rdi, rbx
    mov rax, rdi
    shr rdi, 12
    and rax, 0xFFF
    cmp rax, 0
    je .num_of_ptes_calculation_completed
    inc rdi

.num_of_ptes_calculation_completed:
    mov qword[rdx+0x18], rdi

    add r8, rdi
    mov r9, qword[rdx + 0x10]
    shl r9, 9
    cmp r8, r9
    jna .done_for_num_of_pdes
    mov rdi, qword[rdx + 0x10]
    inc rdi
    mov qword[rdx + 0x10], rdi
.done_for_num_of_pdes:


    mov rsp, rbp
    pop rbp
    pop r8
    pop rbx
    ret


; fill rest of current page with 0
times (ccldr_routine_size) - ($ - $$) db 0



section .data
gdt_start:
    dq 0
gdt_krnl_code:
    ; dq 0x00209A0000000000             ; 64-bit code descriptor (exec/read).
    dw 0xFFFF               ; segment limit 15:00
    dw 0x0                  ; base address 15:00
    db 0                    ; base 23:16

    ;   Present: 1
    ;   DPL: 00
    ;   S: 1 (represent code or data)
    ;   Type: Execute/Read(1010)
    db 0b_1_00_1_1_0_1_0
    ; G: 1 (when flag is set, the segment limit is interpreted in 4-KByte units.)
    ; D/B: (The flag should always be set to 1 for 32-bit code and data segments)
    ; L: 1 (64-bit code segment)
    ; AVL: 0 (available for use by system software)
    ; SegLimit 19:16: 0xF
    db 0b_1_0_1_0_0000 | (0xF)
    db 0                    ; base 31:24
gdt_krnl_data:
    ; dq 0x0000920000000000             ; 64-bit data descriptor (read/write).
    dw 0xFFFF               ; segment limit 15:00
    dw 0x0                  ; base address 15:00
    db 0                    ; base 23:16

    ;   Present: 1
    ;   DPL: 00
    ;   S: 1 (represent code or data)
    ;   Type: Read/Write(0010)
    db 0b_1_00_1_0_0_1_0

    ; G: 1 (when flag is set, the segment limit is interpreted in 4-KByte units.)
    ; D/B: (The flag should always be set to 1 for 32-bit code and data segments)
    ; L: 1 (64-bit code segment)
    ; AVL: 0 (available for use by system software)
    ; SegLimit 19:16: 0xF
    db 0b_1_0_1_0_0000 | (0xF)
    db 0                    ; base 31:24

gdt_end:
    dq 00

gdt_descriptor:
    ;  the GDT limit should always be one less than an integral multiple of eight (that is, 8N � 1).
    dw ((gdt_end - gdt_start) - 1)
    dq 00
