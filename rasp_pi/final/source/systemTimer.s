.globl GetTimerAddr
GetTimerAddr:
    ldr r0, =0x20003000
    mov pc, lr

.globl GetTime
GetTime:
    push {lr}
    bl GetTimerAddr
    ldrd r0, r1, [r0, #4]
    pop {pc}

.globl TimerWait
TimerWait:
    delay .req r2
    mov delay, r0
    push {lr}
    bl GetTime
    start .req r3
    mov start, r0

    loop$:
    bl GetTime
    elapsed .req r1
    sub elapsed, r0, start @//stores the result in elapsed
    cmp elapsed, delay
    .unreq elapsed
    bls loop$

    .unreq delay
    .unreq start
    pop {pc}
