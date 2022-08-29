div:        ; func div(dividend => A, divisor => B) --> (remainder => A, quotient => C)
    ld c,0      ; how many times were we able to subtract

    divloop:
        cmp A,b
        jr c,enddiv	; if A is smaller than B we are done

        inc c		; else we're able to sub once more
        sub A,b		; so sub
        jr divloop	; and repeat

    enddiv:
    ret