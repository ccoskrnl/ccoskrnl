[bits 64]
section .text
global lock_cmpxchg
global lock_cmpxchg16b

; @param[in out]    uint64_t volatile           *destination                A pointer to the destination value.
; @param[in]        uint64_t                    exchange                    The exchange value.
; @param[in]        uint64_t                    comperand                   The value to compare to Destination.

; @retval           uint64_t                    The function returns the initial value of the destination parameter.
lock_cmpxchg:
    mov rcx, qword[rdi]
    mov rax, rdx
    lock cmpxchg qword[rdi], rsi
    mov rax, rcx
    ret


; @param[in out]    uint128_t volatile           *destination                A pointer to the destination value.
; @param[in]        uint128_t                    exchange                    The exchange value.
; @param[in]        uint128_t                    comperand                   The value to compare to Destination.
; @retval           uint128_t                   The functin returs the initial value of the destination parameter.

; rdi:  *destination
; rsi:  exchange.low
; rdx:  exchange.high
; rcx:  comperand.low
; r8:   comperand.high
lock_cmpxchg16b:

    ; exchange: rcx:rbx
    ; comperand: rdx:rax

    mov rbx, rsi
    mov rax, rcx

    mov rcx, rdx
    mov rdx, r8

    lock cmpxchg16b oword [rdi]
    ret