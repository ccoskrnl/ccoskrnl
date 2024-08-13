[bits 64]
section .text

global pit_prepare_sleep
global pit_perform_sleep


; Introducation to Port 0x61

; Bit 0: Controls the gate for PIT Channel 2.
;   It is used to enable or disable the gate for PIT Channel 2. 
;   When this bit is set to 1, it allows the PIT Channel 2 output 
;   to be connected to the speaker or other devices. When cleared (0), 
;   it disables the gate.

; Bit 1: Controls the PC speaker.
;   It controls the state of the PC speaker. When set to 1, the 
;   speaker is enabled, and when cleared (0), the speaker is disabled.

; Bit 2~7: Reserved or used for other purposes.



; Function: prepare for sleep specific number of micorseconds.
; Parameters:
;      edi                                     which holds how many number of microseconds 
;                                              that need to sleep. But it must greater that
;                                              0 and smaller that 55,555.
;                                              55,555 = 1,000,000 / (1193180 / 65535)
; Return value:
;       ST_INVALID_PARAMETER                    Passed one or more invalid parameters.
pit_prepare_sleep:

    cmp edi, 0
    je error                ; Check if it is equal to zero.

    cmp edi, 55555          ; Check if it is less than 55555.
    jb cont                 ; Continue execution.

error:
    mov eax, 2
    ret                     ; Otherwise, we set error value and return immediately.

cont:
    ; Set frequency to specific

    ; prepare for division
    xor rax, rax
    xor rdx, rdx
    
    mov eax, 1000000    ; Set dividend
    div edi             ; The divisor is the parameter passed by caller.
    mov edi, eax        ; Hold the result on edi.

    xor eax, eax

    ; Ditto
    mov eax, 1193180
    div edi
    mov di, ax

    ; Initialize PIT Channel 2 in one-shot mode
    ; Waiting 1 sec could slow down boot time considerably,
    ; so we'll wait 1/100 sec, and multiply the counted ticks

    ; Set up the PIT control register
    mov dx, 0x61        ; Port 0x61 (PIT control port, which controls PIT Channel 2 gate)
    in al, dx
    and al, 0xFC        ; Clear bits 0 and 1
    or al, 1            ; Set bit 0 (enable speaker gate)
    out dx, al

    ; Set PIT Channel 2 to one-shot mode
    mov al, 0xB2        ; Command byte: Channel 2, Access mode lobyte/hibyte, Mode 2 (rate generator), Binary mode
    out 0x43, al        ; Send command byte to PIT command port

    mov al, dil         ; Low byte of divisor
    out 0x42, al        ; Send low byte to Channel 2 data port
    shr di, 8
    mov al, dil         ; High byte of divisor
    out 0x42, al        ; Send high byte to Channel 2 data port
    xor rax, rax
    ret

; Function: perform for sleep specific number of micorseconds.
pit_perform_sleep:

    ; Reset PIT one-shot counter
    mov dx, 0x61
    in al, dx
    and al, 0xFE        ; Clear bit 0 (disable speaker gate)
    out dx, al
    or al, 1            ; Set bit 0 (enable speaker gate)
    out dx, al

    ; Now wait until PIT counter reaches zero
.wait:
    in al, dx
    and al, 0x20        ; Check bit 5 (PIT output)
    jz .wait            ; Loop until bit 5 is set

    ret