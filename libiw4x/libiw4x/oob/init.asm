BITS 64
DEFAULT REL

; Import C interface dispatcher.
;
EXTERN cl_dispatch_connectionless_packet

SECTION .text

; Export stub dispatcher.
;
GLOBAL cl_dispatch_connectionless_packet_stub

cl_dispatch_connectionless_packet_stub:
    ; Save return address.
    ;
    push    rax
    mov     rax, [rsp+8]
    mov     [rsp-8], rax
    pop     rax

    ; Save all registers.
    ;
    push    rax
    push    rcx
    push    rdx
    push    rbx
    push    rbp
    push    rsi
    push    rdi
    push    r8
    push    r9
    push    r10
    push    r11
    push    r12
    push    r13
    push    r14
    push    r15

    ; Allocate shadow space and align stack.
    sub     rsp, 0x28

    ; Setup parameters.
    mov     rcx, rbx  ; RCX = netadr_t* (sender address)
    mov     rdx, r15  ; RDX = msg_t* (message)

    ; Call C wrapper.
    call    cl_dispatch_connectionless_packet

    ; Save result.
    mov     cl, al

    ; Restore stack.
    ;
    add     rsp, 0x28

    ; Restore all registers.
    ;
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     r11
    pop     r10
    pop     r9
    pop     r8
    pop     rdi
    pop     rsi
    pop     rbp
    pop     rbx
    pop     rdx
    pop     rcx
    pop     rax

    ; Check if handler processed the packet.
    ;
    test    cl, cl
    jz      .handler_not_invoked

    ; Handler processed the packet.
    ;
    ; Return directly to caller.
    ;
    mov     al, 1
    ret

.handler_not_invoked:
    ; No handler processed the packet.
    ;
    ; Resume original control flow.
    ;
    mov     r11, 0x1400F6065  ; Address after our 14-byte hook.
    jmp     r11
