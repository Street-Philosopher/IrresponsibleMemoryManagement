ld a,255    ; dividendo
ld C,255    ; divisore

ld B,0      ; reset b

divloop:
cmp A,C
jc enddiv

inc B
sub A,C
jp divloop


enddiv:
; resto è in A, quoziente è in B
stop


; N
; D

; N - D
; N < D

; T => quoziente
; N => resto