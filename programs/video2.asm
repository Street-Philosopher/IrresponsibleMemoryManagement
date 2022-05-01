ld a,0
ld hl,0
ld b,32

loop:
    stv (HL+),a
    inc a
    jc nextloop
    jp loop

nextloop:
    dec b
    jnz loop

stop