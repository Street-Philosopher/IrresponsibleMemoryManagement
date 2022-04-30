ld d,10     ; numbers up to this will be computed

ld b,0
ld c,1

fibloop:
    ld a,c
    add a,b
    ld b,c
    ld c,a

    push a

    dec d
    jnz fibloop

stop