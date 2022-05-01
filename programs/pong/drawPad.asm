#def FULL_BLACK as 0
#def FULL_WHITE as 0xff


; position goes in Bm initial address in HL
drawPad:
	ld D,0xff

	ld C,8			; every 8 we have to subtract 256 instead of just decreasing so we need this

	ld A,FULL_BLACK
	drawBlacLoop1:
		stv (HL-),A
		dec C
		jnz normalHLDecLoop1

		ld C,8	; restart the count
		sub HL,248

		normalHLDecLoop1:
		dec D
		dec B
		jnz drawBlacLoop1
		
	ld B,PLAYER_HEIGHT
	ld A,FULL_WHITE
	drawWhiteLoop1:
		stv (HL-),A
		dec C
		jnz normalHLDecLoop2

		ld C,8	; restart the count
		sub HL,248

		normalHLDecLoop2:
		dec D
		dec B
		jnz drawWhiteLoop1

	ld A,FULL_BLACK
	drawBlacLoop2:
		stv (HL-),A
		dec C
		jnz normalHLDecLoop3

		ld C,8	; restart the count
		sub HL,248

		normalHLDecLoop3:
		dec D
		jnz drawBlacLoop2

	ret
