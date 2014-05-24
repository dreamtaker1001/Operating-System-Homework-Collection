.section .init
.globl _start
_start:
    b main

.section .text
main:
    mov sp, #0x8000
@set the function select region
    pinNum .req r0
    pinFunc .req r1
    mov pinNum, #16
    mov pinFunc, #1
    bl SetGpioFunction
    .unreq pinNum
    .unreq pinFunc

ptn1$:
    ptrn .req r4
    ldr ptrn, =pattern1
    ldr ptrn, [ptrn]
    seq .req r5
    mov seq, #0
    bl loop$
ptn2$:
    ptrn .req r4
    ldr ptrn, =pattern2
    ldr ptrn, [ptrn]
    seq .req r5
    mov seq, #0
    bl loop$
ptn3$:
    ptrn .req r4
    ldr ptrn, =pattern3
    ldr ptrn, [ptrn]
    seq .req r5
    mov seq, #0
    bl loop$
    b ptn1$
loop$:
    pinNum .req r0
    pinVal .req r1
    mov pinNum, #16
    mov pinVal, #1
    lsl pinVal, seq  @//seq = 0, 1, 2, ...
    and pinVal, ptrn
    .unreq pinNum
    .unreq pinVal
    push {lr}
    bl SetGpio

    ldr r0, =250000
    bl TimerWait

    pop {lr}
    add seq, #1   @// seq++
    cmp seq, #0b11111
    bls loop$
    .unreq seq
    .unreq ptrn
    mov pc, lr

.section .data
.align 2
pattern1:
.int 0b10001010001010101010101000101000
.align 2
pattern2:
.int 0b01010100010101010100011111111110
.align 2
pattern3:
.int 0b11111111111111110001000101010101
