[bits 64]
section .text
global outb
global outw
global inb
global inw

; Function: outb - Output a byte to an I/O port
; Parameters:
;   dil - The data byte to output
;   si - The I/O port address
outb:
    mov al, dil
    mov dx, si
    out dx, al
    ret


; Function: outw - Output a word to an I/O port
; Parameters:
;   di - The data word to output
;   si - The I/O port address
outw:
    mov ax, di
    mov dx, si
    out dx, ax
    ret


; Function: inb - Input a byte from an I/O port
; Parameters:
;   di - The I/O port address
; Returns:
;   al - The data byte read from the I/O port
inb:
    mov dx, di
    in al, dx
    ret


; Function: inw - Input a word from an I/O port
; Parameters:
;   di - The I/O port address
; Returns:
;   ax - The data word read from the I/O port
inw:
    mov dx, di
    in ax, dx
    ret
