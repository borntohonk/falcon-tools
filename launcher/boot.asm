.section #payload 0xF00
_start:
    mov $r10 0x1250
    mov $sp $r10
    
    add $sp -0x80 // Allocate space on the stack for the key_buffer
    mov $r5 $sp // key_buffer

    mov $r10 0xFFAABBFF
    lcall #write_mbox1

    mov b32 $r10 $r5 // dst
    mov $r11 0x300 // src
    mov $r12 0x7C // len
    lcall #memcpy_i2d

    clear b32 $r10 // dst
    mov $r11 0x400 // src
    ld b32 $r12 D[$r5 + 0x74] // len
    lcall #memcpy_i2d

    mov $r10 0x300 // dst
    clear b32 $r11 // src
    ld b32 $r12 D[$r5 + 0x74] // size
    mov $r13 0x1 // is_secret
    lcall #memcpy_d2i

    ld b32 $r9 D[$r5 + 0x74] // blob1_size
    shl b32 $r9 0x10
    or $r9 0x3

    mov b32 $r8 $r5
    add b32 $r8 0x20
    mov $r7 0xFFF60000
    or $r7 $r7 $r8
    clear b32 $r8

    mov b32 $r10 $r9
    lcall #write_mbox0

    mov b32 $r10 $r7
    lcall #write_mbox1 

    mov $cauth $r9
    cxset 0x2
    xdst $r8 $r7
    xdwait

    mov $r8 #success
    // Keygenldr args
    mov b32 $r10 $r5
    mov $r11 0x69
    clear b32 $r12

    // Keygenldr
    lcall 0x300
success:
    lcall #write_mbox0

    mov $r10 0xF100F
    lcall #write_mbox1
exit:
    exit
    bra b #exit

/*
    r10 - dst
    r11 - val
    r12 - size
*/
memset:
    bra #memset_check
memset_loop:
    st b32 D[$r10] $r11
    add b32 $r10 4
    sub b32 $r12 4
memset_check:
    bra b32 $r12 0 ne #memset_loop
    ret

/*
    r10 - dest
    r11 - src
    r12 - size

    pollutes: r9
*/
memcpy:
    bra #mcpy_check
mcpy_loop:
    ld b32 $r9 D[$r11]
    st b32 D[$r10] $r9
    add b32 $r10 4
    add b32 $r11 4
    sub b32 $r12 4
mcpy_check:
    bra b32 $r12 0 ne #mcpy_loop
    ret


/* 
    r10 - dest
    r11 - src
    r12 - size
*/
memcpy_i2d:
    mov $r9 0x2000000
    or $r11 $r11 $r9
    mov $r8 0x6000
    mov $r9 0x6100
    iowr I[$r8] $r11
memcpy_i2d_loop:
    iord $r15 I[$r9]
    st b32 D[$r10] $r15
    sub b32 $r12 4
    add b32 $r10 4
memcpy_i2d_check_remaining:
    cmp b32 $r12 0
    bra ne #memcpy_i2d_loop
    ret

/* 
    r10 - dest
    r11 - src
    r12 - size
    r13 - secret
*/
memcpy_d2i:
    mov $r9 0x1000000
    bra b8 $r11 0x0 ne #md2i_invalid_param
    bra b8 $r12 0x0 ne #md2i_invalid_param
    cmp b32 $r13 0
    bra e #md2i_not_secret
    mov $r9 0x11000000
md2i_not_secret:
    or $r9 $r10 $r9
    mov $r15 0x6000
    iowr I[$r15] $r9
    mov $r8 0x6100
    mov $r9 0x6200
md2i_write_loop_check_new_vpage:
    and       $r7 $r10 0xff
    bra       z #md2i_write_loop_new_vpage
md2i_write_loop:
    ld b32    $r1 D[$r11]
    ld b32    $r2 D[$r11 + 0x4]
    ld b32    $r3 D[$r11 + 0x8]
    ld b32    $r4 D[$r11 + 0xc]
    iowr      I[$r8] $r1
    iowr      I[$r8] $r2
    iowr      I[$r8] $r3
    iowr      I[$r8] $r4
    add b32   $r10 0x10
    add b32   $r11 0x10
    sub b32   $r12 0x10
md2i_check_remaining:
    cmp b32 $r12 0x0
    bra ne #md2i_write_loop_check_new_vpage
    ret
md2i_write_loop_new_vpage:
    shr b32   $r7 $r10 0x8
    iowr      I[$r9] $r7
    bra      #md2i_write_loop
md2i_invalid_param:
    bra #md2i_invalid_param

/* 
    r10 - value
*/
write_mbox0:
    mov $r1 0x1000
    iowr I[$r1] $r10
    ret

/* 
    r10 - value
*/
write_mbox1:
    mov $r1 0x1100
    iowr I[$r1] $r10
    ret


exit
exit
exit
exit
