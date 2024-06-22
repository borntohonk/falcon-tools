.section #ns_code

// Set up the Falcon stack pointer.
mov $r13 #FALCON_HWCFG
iord $r13 I[$r13]
shr b32 $r13 0x9
and $r13 0x1FF
shl b32 $r13 0x8
mov $sp $r13
lcall #main
exit

pushdef(`key_data_addr', `$r5')

main:
    mov $r15 -0x10
    add $sp -0x11C

    mpush $r8

    // Allocate memory for the Key Data table.
    mov $r9 $sp
    add b32 $r9 $r9 0xC4
    and key_data_addr $r9 $r15

    // Copy Key Data into DMEM.
    mov b32 $r10 key_data_addr
    mov $r11 #KEY_TABLE_START
    mov $r12 #KEY_TABLE_SIZE
    lcall #memcpy_i2d

    // Copy the signed microcode portion to DMEM.
    clear b32 $r10
    mov $r11 #HS_PAYLOAD_PHYS_ADDR
    ld b32 $r12 D[key_data_addr + 0x20]
    lcall #memcpy_i2d

    // Remap the signed microcode and tag it as secure.
    mov b32 $r11 $r10
    mov $r10 #HS_PAYLOAD_START
    mov b32 $r13 $r10
    mov $r14 0x1
    lcall #memcpy_d2i

    // Transfer the MAC of the secure payload into crypto register 6.
    clear b32 $r7
    mov b32 $r8 key_data_addr
    sethi $r8 0x60000
    cxset 0x2
    xdst $r7 $r8
    xdwait

    // Transfer the seed for the fake-signing key into crypto register 7.
    clear b32 $r7
    add b32 $r8 key_data_addr 0x10
    sethi $r8 0x70000
    cxset 0x2
    xdst $r7 $r8
    xdwait

    // Load in the cauth details for Heavy Secure mode authentication.
    ld b32 $r9 D[key_data_addr + 0x20]
    shl b32 $r9 0x10
    mov $r15 #HS_PAYLOAD_START
    shr b32 $r15 0x8
    or $r9 $r15
    mov $cauth $r9

    // Jump to Heavy Secure Mode!
    lcall #HS_PAYLOAD_START

    mpopaddret $r8 0x11C

popdef(`key_data_addr')

include(`base_define.asm')
include(`mmio.asm')
include(`memcpy.asm')
include(`memcpy_i2d.asm')
include(`memcpy_d2i.asm')
include(`tsec_wait_dma.asm')
include(`tsec_dma_write.asm')
include(`tsec_set_key.asm')

.align 0x100
.section #ns_data 0x200
.equ #KEY_TABLE_SIZE 0x7C

KEY_TABLE_START:
HS_PAYLOAD_MAC:     .skip 0x10
HS_PAYLOAD_SEED:    .skip 0x10
HS_PAYLOAD_SIZE:    .b32 0x00000000

.align 0x100
HS_PAYLOAD_PHYS_ADDR:
.section #hs_code 0x200
.equ #HS_PAYLOAD_START 0x200

hs_main:
    mov $r11 #FALCON_MAILBOX1
    mov $r12 0xBADC0DED
    iowr I[$r11] $r12
    csecret $c0 0x0 // this is all that is needed + cxset xdld to get acl 0x13 secrets
    csigenc $c0 $c0
    cxset 0x2 // changes target for next operation to cryto instead of DMA
    xdld $r10 $r10 // load from crypto to $r10, $r10 is equivelant of $c0
    xdwait // waits for xdld with cxset to complete
    lcall #tsec_set_key // this takes what is stored in $r10 and pushes it through sor1
    csigclr
    ret

.align 0x100
