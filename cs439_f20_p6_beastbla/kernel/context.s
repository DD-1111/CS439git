    .global x_contextSwitch
    // x_contextSwitch(SaveArea* from, SaveArea* to)
    //
    // 8 registers, 32 bit each
    // caller-saved (%EAX, %ECX, %EDX)
    // callee-saved (%EBX, %ESI, %EDI, %EBP, %ESP)
    // arguments are passed on the stack
    //        argument 0      4(%ESP)
    //        argument 1      8(%ESP)
    //        ...
    // return value: %EAX, %EDX
x_contextSwitch:
    mov 4(%esp),%eax        # eax -> from SaveArea
    mov 8(%esp),%ecx        # ecx -> to saveArea

    // Save context (callee-saved of the "from" thread)
    mov %ebx,0(%eax)     # mem32[%eax + 0] = %ebx
    mov %esp,4(%eax)
    mov %ebp,8(%eax)
    mov %esi,12(%eax)
    mov %edi,16(%eax)
    // change the ready flag
    movl $1, 20(%eax)

    // Load context (callee-saved of the "to" thread)
    mov 0(%ecx),%ebx
    mov 4(%ecx),%esp    # <----------- 
    mov 8(%ecx),%ebp
    mov 12(%ecx),%esi
    mov 16(%ecx),%edi

    ret