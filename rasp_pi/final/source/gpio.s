.globl GetGpioAddress
GetGpioAddress:
    gpioAddr .req r0
    ldr r0, =0x20200000
    mov pc, lr
    .unreq gpioAddr

.globl SetGpioFunction
SetGpioFunction:
    pinNum .req r0
    pinFunc .req r1
    cmp pinNum, #53
    cmpls pinFunc, #7
    movhi pc, lr @//invalid value, should return
    push {lr} @//save the return addr for later
    mov r2, pinNum @//Protect r0: pin number
    bl GetGpioAddress
    gpioAddr .req r0
@//R2 is now the pin number
    functionLoop$:  @// r0 <= r0 + 4(r2/10)
        cmp pinNum, #9
        subhi pinNum, #10
        addhi gpioAddr, #4
        bhi functionLoop$
@//Now, R2 contains the remainder
@//and R0 is the GPIO addr.
    add pinNum, pinNum,lsl #1   @//r2 <- r2*3
    lsl pinFunc, pinNum   @//r1 <- r1 left shift r2 bits.
    .unreq pinNum
    str r1, [r0]  @//r1 is the final GPIO address.
    pop {pc}
    .unreq pinFunc
    .unreq gpioAddr

.globl SetGpio
SetGpio:
    pinNum .req r0
    pinVal .req r1
    cmp pinNum, #53
    movhi pc, lr
    push {lr}
    mov r2, pinNum
    .unreq pinNum
    pinNum .req r2
    bl GetGpioAddress
    gpioAddr .req r0
    pinBank .req r3
    lsr pinBank, pinNum, #5  @// pinBank <- pinNum/32
    lsl pinBank, #2  @// pinBank <- pinBank * 4
    add gpioAddr, pinBank @//gpioAddr is either 20200000H
               @// or 20200004H
    .unreq pinBank
    and pinNum, #31  @//set pinNum=num & 11111b
    setBit .req r3
    mov setBit, #1  @//set bit = 1b
    lsl setBit, pinNum  @// bit << num
    .unreq pinNum
    teq pinVal, #0  @//test whether pinVal is 0
    .unreq pinVal
    streq setBit, [gpioAddr, #40] @//if 0, off
    strne setBit, [gpioAddr, #28] @//if 1, on
    .unreq setBit
    .unreq gpioAddr
    pop {pc} @//return
