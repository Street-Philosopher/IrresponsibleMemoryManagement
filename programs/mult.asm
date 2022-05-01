ld A,239
ld (0x1000),A       ; two arguments are stored
ld A,12
ld (0x1001),A

mulstart:
ld D,(0x1000)
jnz initcont
jp hlstorage        ; if D is zero continuing would cause the result to be 0x100. this way it works
initcont:
ld C,(0x1001)       ; load the arguments and reset A
xor A,A

addingloop:
add A,C                 ; add one argument and decrease the other
call c,carryhandling    ; this adds one to B, which will be the highest
dec D
jnz addingloop

hlstorage:
ld L,A
ld A,B      ; store the result in HL
ld H,A

stop        ; done!

carryhandling:
inc B
ret