; this program assembles to the file of bytes you find in 'programs/primes'

ld d,0          ; this will only be used for comparisons
ld a,1          ; dividend
ld HL,0x1000    ; pointer to the area of memory where we'll store the primes we found


numloop:
ld b,1          ; divisor
inc a
jc end
ld (0x2000),A   ; save current dividend as A is changed by the "div" function

dvdloop:
ld a,(0x2000)
inc b				; next divisor
jc enddvdloop       ; except if we carry, which means we are out divisors for this number

cmp a,b         ; we repeat until the dividend and the divisor are the same. after that, no point in checking
jz enddvdloop   ; this will only jump if the two values were equal

call div
cmp a,d         ; if mod is 0
jz nextnum      ; ignore this number as it's not prime

jp dvdloop      ; else, go to the next divisor

enddvdloop:
ldi (HL),A      ; this is reached when we are out of divisors for the dividend
				; it loads the current number to memory and increases the pointer by one, so that we'll be pointing to a different area

nextnum:
ld a,(0x2000)		; get the dividend
jp numloop

end:
stop



div:
ld c,0      ; how many times were we able to subtract

divloop:
cmp A,b
jc enddiv	; if A is smaller than B we are done

inc c		; else we're able to sub once more
sub A,b		; so sub
jp divloop	; and repeat


enddiv:
; once this is reached remainder is in A, quotient in C
ret