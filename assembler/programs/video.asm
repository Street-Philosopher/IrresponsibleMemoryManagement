ld hl,part1

; lo stack sarà così:
;   0xFC-0xFD: puntatore alla vram
;   0xFE-0xFF: puntatore alla ram

ld a,0
ld (0xfffc),a
ld (0xfffd),a

ld c,4
outerloop:
ld d,8
loop:
    ldi a,(hl)
    push hl     ; salva il valore in ram
    
    dec sp
    dec sp      ; sposta lo stack alla parte col puntatore vram
    pop hl      ; prendi il puntatore vram

    stv (hl+),a
    push hl     ; salva il nuovo valore vram

    inc sp
    inc sp      ; riporta sul puntatore alla ram
    pop hl      ; salva il puntatore ram

    dec d
    jnz loop

; quando si arriva qui lo stack è su FF, e HL contiene il puntatore alla ram

ld a,c
cmp a,3
jp nz,ignoreHLIncrease

dec sp
dec sp      ; sposta lo stack sulla vram
dec sp      ; lo facciamo quattro volte perché nel loop sopra c'è anche push, qui no
dec sp
pop hl

; ora in hl c'è il puntatore in vram, e lo stack è su FD

ld b,240
ld a,0      ; in questo modo aggiungiamo 256
add hl,ab
push hl

; stack su FB

inc sp
inc sp
pop hl

; stack su FF


ignoreHLIncrease:
dec c
jnz outerloop

end:
jp end

stop


part1:
db 7 ,24,32,64,78,152,128,128
part2:
db 224,24,4,2,122,9,1,1
part3:
db 0b10000000,0b10010000,0b10001000,0b01001100,0b01000111,0b00100000,0b00011000,0b00000111
part4:
db 1,9,0b11001,0b110010,0b11000010,0b100,0b11000,0b11100000