; load the VRAM with black tiles
cls:
	ld B,0x20
	ld A,FULL_BLACK
	ld HL,0x0000
	clsLoop2:
		ld C,0x00
		clsLoop1:
			stv (HL+),A
			dec C
			jr nz, clsLoop1
		dec B
		jr nz, clsLoop2
	ret


drawPadsAndBall:

	drawBall:
		ld A,(ballXPos)
		ld B,8
		call div		; after this A has the remainder and B the quotient

		; after this switching the remainer is in B and the quotient in A bc we need it later
		ld B,A
		ld A,C

		; C will be the left side of the ball, D the right
		ld C,0xff
		ld D,0x00

		inc B	; this is stupid
		dec B	; we check if B is 0 without cmp
		ballXValueLoop:
			jr z, ballXValueLoop_end

			ror C	; this sets the carry	; 	while (B--)
			ror D	; this uses the carry	; 		C = C >> 1
											; 		D = (D >> 1) + (1 << 7)
			dec B
			jr ballXValueLoop
		ballXValueLoop_end:

		; C and D will be changed so do this to save them
		ld (tempBallAddrStorage_C),C
		ld (tempBallAddrStorage_D),D

		; now C and D contain the things to draw on the X. now we need to draw smthing similar to the pads but smaller and using these things instead
		ld B,A
		ld A,7		;bottom row

		inc B
		dec B
		drawballAddrFindLoop:
			jr z, drawballAddrFindLoop_end

			add A,8
			dec B
			jr drawballAddrFindLoop
		drawballAddrFindLoop_end:
		ld B,A
		ld A,0x1F

		ld HL,AB
		push HL		; we need this later to draw the seocnd column


		; yes this code is the same as the function above
		; draw leftmost column of ball
		drawLeftPart:
		ld B,(ballYPos)

		ld C,8			; every 8 we have to subtract 256 from HL instead of just decreasing so we need this

		; get to the address we want
		drawBlacLoop1_ball_left:
			dec HL
			dec C
			jr nz, normalHLDecLoop1_ball_left

			ld C,8	; restart the count
			sub HL,248

			normalHLDecLoop1_ball_left:
			dec D
			dec B
			jr nz, drawBlacLoop1_ball_left
			
		ld B,8
		ld A,(tempBallAddrStorage_C)
		drawBallLeftLoop:
			stv (HL-),A
			dec C
			jr nz, normalHLDecLoop2_ball_left

			ld C,8	; restart the count
			sub HL,248

			normalHLDecLoop2_ball_left:
			dec D
			dec B
			jr nz, drawBallLeftLoop
		
		; draw right part of the ball
		drawRightPart:
		ld B,(ballYPos)

		; this way we go to the next column
		pop hl
		add hl,8

		ld C,8			; every 8 we have to subtract 256 instead of just decreasing so we need this

		drawBlacLoop1_ball_right:
			dec HL
			dec C
			jr nz, normalHLDecLoop1_ball_right

			ld C,8	; restart the count
			sub HL,248

			normalHLDecLoop1_ball_right:
			dec D
			dec B
			jr nz, drawBlacLoop1_ball_right
			
		ld B,8
		ld A,(tempBallAddrStorage_D)
		drawBallRightLoop:
			stv (HL-),A
			dec C
			jr nz, normalHLDecLoop2_ball_right

			ld C,8	; restart the count
			sub HL,248

			normalHLDecLoop2_ball_right:
			dec D
			dec B
			jr nz, drawBallRightLoop

	
	P1DrawPad:
	ld B,(p1Position)
	ld HL,0x1F0F	; lowest row of second column
	call drawPad


	P2DrawPad:
	ld B,(p2Position)
	ld HL,0x1FF7	; lowest row of second to last column
	call drawPad
	

	ret

drawScore:
	ld HL,image_zero
	ld B,(p1Score)
	inc B
	imageCalcLoop_p1:
		dec B
		jr z, endLoopImageCalc_p1
		add HL,8
		jr imageCalcLoop_p1
	endLoopImageCalc_p1:
	ld AB,HL
	ld HL,P1_SCORE_ADDR
	ccr (HL),(AB)

	ld HL,image_zero
	ld B,(p2Score)
	inc B
	imageCalcLoop_p2:
		dec B
		jr z, endLoopImageCalc_p2
		add HL,8
		jr imageCalcLoop_p2
	endLoopImageCalc_p2:
	ld AB,HL
	ld HL,P2_SCORE_ADDR
	ccr (HL),(AB)
	ret



; position goes in Bm initial address in HL
drawPad:
	ld D,0xff

	ld C,8			; every 8 we have to subtract 256 instead of just decreasing so we need this

	ld A,FULL_BLACK
	drawBlacLoop1:
		stv (HL-),A
		dec C
		jr nz, normalHLDecLoop1

		ld C,8	; restart the count
		sub HL,248

		normalHLDecLoop1:
		dec D
		dec B
		jr nz, drawBlacLoop1
		
	ld B,PLAYER_HEIGHT
	ld A,FULL_WHITE
	drawWhiteLoop1:
		stv (HL-),A
		dec C
		jr nz, normalHLDecLoop2

		ld C,8	; restart the count
		sub HL,248

		normalHLDecLoop2:
		dec D
		dec B
		jr nz, drawWhiteLoop1

	ld A,FULL_BLACK
	drawBlacLoop2:
		stv (HL-),A
		dec C
		jr nz, normalHLDecLoop3

		ld C,8	; restart the count
		sub HL,248

		normalHLDecLoop3:
		dec D
		jr nz, drawBlacLoop2

	ret
