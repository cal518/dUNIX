BITS 64
GLOBAL start
EXTERN k_main

section .text
start:
    cli
    mov rsp, stack_top
    call k_main
.hang: hlt
    jmp .hang

section .bss
resb 4096
stack_bottom:
    resb 16384
stack_top:
