ld a,(next_section)

nop
halt
push HL
push AB
pop HL
pop AB

add A,A
add A,B
add A,C
add A,D
sub A,A
sub A,B
sub A,C
sub A,D
add A,0xFF
sub A,0xFF
add HL,2
sub HL,2
and A,0xFF
and A,B
and A,C
and A,D
xor A,A
xor A,B
xor A,C
xor A,D
or  A,0xFF
or  A,B
or  A,C
or  A,D
inc A
inc B
inc C
inc D
dec A
dec B
dec C
dec D
inc HL
inc SP
dec HL
dec SP
cmp A,0xFF
cmp A,B
cmp A,C
cmp A,D
not A
not B
not C
not D
ror A
ror B
ror C
ror D
rol A
rol B
rol C
rol D
add HL,AB
adc A,A
adc A,B
adc A,C
adc A,D

bit 0,A
bit 1,A
bit 2,A
bit 3,A
bit 4,A
bit 5,A
bit 6,A
bit 7,A
res 0,A
res 1,A
res 2,A
res 3,A
res 4,A
res 5,A
res 6,A
res 7,A
set 0,A
set 1,A
set 2,A
set 3,A
set 4,A
set 5,A
set 6,A
set 7,A
flag 2
flag 3
flag 4
flag 5
flag 6
flag 7

ld A,B
ld A,C
ld A,D
ld B,A
ld B,C
ld B,D
ld C,A
ld C,B
ld C,D
ld D,A
ld D,B
ld D,C
ld A,0
ld B,1
ld C,2
ld D,3
ld A,H
ld A,L
ld H,A
ld L,A
ld A,(HL)
ld B,(HL)
ld C,(HL)
ld D,(HL)
ld (HL),A
ld (HL),B
ld (HL),C
ld (HL),D
ld HL,0x55AA
ld HL,AB
ld AB,HL
ld (0x8000),A
ld (0x8000),B
ld (0x8000),C
ld (0x8000),D
ld A,(0x8000)
ld B,(0x8000)
ld C,(0x8000)
ld D,(0x8000)
ldi A,(HL)
ldi B,(HL)
ldi C,(HL)
ldi D,(HL)
ldi (HL),A
ldi (HL),B
ldi (HL),C
ldi (HL),D
ldd A,(HL)
ldd B,(HL)
ldd C,(HL)
ldd D,(HL)
ldd (HL),A
ldd (HL),B
ldd (HL),C
ldd (HL),D

stv (0),A
stv (0),B
stv (0),C
stv (0),D
ldv A,(0)
ldv B,(0)
ldv C,(0)
ldv D,(0)
stv (HL),A
stv (HL+),A
stv (HL-),A
ldv A,(HL)
ccv (HL),(0)
ccr (HL),(0)
ccv (HL),(AB)
ccr (HL),(AB)

ldh (1),A
ldh A,(0)

jp second_jump
second_jump:
jp z,third_jump
third_jump:
jp c,fourth_jump
fourth_jump:
jp nz,next_section
next_section:

ld hl, j1
jp (HL)
j1:
ld hl, j2
jp z,(HL)
j2:
ld hl, j3
jp c,(HL)
j3:
ld hl, j4
jp nz,(HL)
j4:
call ret_yes
call z,ret_z
call c,ret_c
call nz,ret_nz

ld hl,ret_yes
call (HL)

ld hl,ret_z
;set zero flag
ld a,1
dec A
call z,(HL)

ld hl,ret_c
;set carry flag
ld a,0
dec A
call c,(HL)

ld hl,ret_nz
;reset zero flag
ld a, 1
inc A
call nz,(HL)

stop

ret_yes:
    ret
ret_z:
    ;set zero flag
    ld a,1
    dec A
    ret z
ret_c:
    ret c
ret_nz:
    ret nz
