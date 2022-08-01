; my respect for people who coded entire games in assembly is immeasurable



#def PLAYER_HEIGHT as 48

#def PLAYER_Y_MAX as 206
#def PLAYER_Y_MIN as 16

#def PLAYER_SPEED as 8			; pixels per cycle

#def BALL_P1_CONTACT as  24
#def BALL_P2_CONTACT as 232
#def BALL_Y_MAX as 240
#def BALL_Y_MIN as  16
#def SCORED_TO_P1 as  16
#def SCORED_TO_P2 as 240
#def BALL_START_YPOS as 124
#def BALL_RESTART_POS_P1 as 24
#def BALL_RESTART_POS_P2 as 232

#def BALL_SPEED as 8			; pixels per cycle
#def DIR_TOP_RIGHT as 0
#def DIR_BTM_RIGHT as 1
#def DIR_BTM_LEFT as 2
#def DIR_TOP_LEFT as 3

#def FULL_BLACK as 0x00
#def FULL_WHITE as 0xFF

; values for movement of the pads
#def SPEED_NO as 0
#def SPEED_UP as 1
#def SPEED_DOWN as 2

#def CRAM_INPUT as 0x80

#def FULL_BLACK as 0x00
#def FULL_WHITE as 0xff

#def P1_SCORE_ADDR as 0x0168
#def P2_SCORE_ADDR as 0x0188
#def PWON_MSG_ADDR as 0x0870

#def SCORE_WIN as 2


mainloop:

	call movePlayers

	call moveBall

	; this is terribly fucking slow
	call cls
	call drawPadsAndBall
	call drawScore

	; now we check if either player has scored a goal
	; first we check if the ball is in player1's thing
	ld A,(ballXPos)
	cmp A,SCORED_TO_P1				;	if ball.pos.x < p1.goalPosition
	call c,player2ScoredRoutine		;		player2ScoredRoutine()

	; then we check for player2's thing
	ld A,SCORED_TO_P2
	ld B,(ballXPos)
	cmp A,B							;	if p2.goalPosition < ball.pos.x
	call c,player1ScoredRoutine		;		player1ScoredRoutine()

	halt
	jp mainloop




; INCLUDING 'drawing.asm':
; ------------------------------
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
			jnz clsLoop1
		dec B
		jnz clsLoop2
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
			jz ballXValueLoop_end

			ror C	; this sets the carry	; 	while (B--)
			ror D	; this uses the carry	; 		C = C >> 1
											; 		D = (D >> 1) + (1 << 7)
			dec B
			jp ballXValueLoop
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
			jz drawballAddrFindLoop_end

			add A,8
			dec B
			jp drawballAddrFindLoop
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
			jnz normalHLDecLoop1_ball_left

			ld C,8	; restart the count
			sub HL,248

			normalHLDecLoop1_ball_left:
			dec D
			dec B
			jnz drawBlacLoop1_ball_left
			
		ld B,8
		ld A,(tempBallAddrStorage_C)
		drawBallLeftLoop:
			stv (HL-),A
			dec C
			jnz normalHLDecLoop2_ball_left

			ld C,8	; restart the count
			sub HL,248

			normalHLDecLoop2_ball_left:
			dec D
			dec B
			jnz drawBallLeftLoop
		
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
			jnz normalHLDecLoop1_ball_right

			ld C,8	; restart the count
			sub HL,248

			normalHLDecLoop1_ball_right:
			dec D
			dec B
			jnz drawBlacLoop1_ball_right
			
		ld B,8
		ld A,(tempBallAddrStorage_D)
		drawBallRightLoop:
			stv (HL-),A
			dec C
			jnz normalHLDecLoop2_ball_right

			ld C,8	; restart the count
			sub HL,248

			normalHLDecLoop2_ball_right:
			dec D
			dec B
			jnz drawBallRightLoop

	
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
		jz endLoopImageCalc_p1
		add HL,8
		jp imageCalcLoop_p1
	endLoopImageCalc_p1:
	ld AB,HL
	ld HL,P1_SCORE_ADDR
	ccr (HL),(AB)

	ld HL,image_zero
	ld B,(p2Score)
	inc B
	imageCalcLoop_p2:
		dec B
		jz endLoopImageCalc_p2
		add HL,8
		jp imageCalcLoop_p2
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
; ------------------------------




; INCLUDING 'players.asm':
; ------------------------------
; check for input and change positions accordingly
movePlayers:
	ldh A,(CRAM_INPUT)

	checkP1Input:
	checkForA_p1:
	bit 6,A						; 	if is_pressed(Key_A)
	call nz,p1_tryingMoveUp		; 		p1.move(up)
	ldh A,(CRAM_INPUT)			; 	else if is_pressed(Key_Z)
	checkForZ_p1:				; 		p1.move(down)
	bit 4,A						; 	else
	call nz,p1_tryingMoveDown	;		nop;

	ldh A,(CRAM_INPUT)

	; same code as above except checks for player 2, using up and down keys
	checkP2Input:
	checkForA_p2:
	bit 2,A
	call nz,p2_tryingMoveUp
	ldh A,(CRAM_INPUT)
	checkForZ_p2:
	bit 3,A
	call nz,p2_tryingMoveDown

	endInputCheck:
	ret

	; these are sub-sub-routines called by this function

	; check if the player1 can move up ad if it can do it
	p1_tryingMoveUp:
		ld A,PLAYER_Y_MAX
		ld B,(p1Position)
		cmp A,B					; 	if not maxPosition < p1Position
		jc end_p1TryUp			; 		p1Position += speed
		ld A,B
		add A,PLAYER_SPEED	; change position and update it
		ld (p1Position),A

		end_p1TryUp:
		ret
	
	; check if the player1 can move down ad if it can do it
	p1_tryingMoveDown:
		ld A,(p1Position)
		cmp A,PLAYER_Y_MIN		; 	if not p1Position < minPosition
		jc end_p1TryDown		; 		p1Position -= speed
		sub A,PLAYER_SPEED
		ld (p1Position),A

		end_p1TryDown:
		ret

	; check if the player2 can move up ad if it can do it
	p2_tryingMoveUp:
		ld A,PLAYER_Y_MAX		; 	if p2Position < maxPosition
		ld B,(p2Position)		; 		p2Position += speed
		cmp A,B
		jc end_p2TryUp
		ld A,B
		add A,PLAYER_SPEED	; change position and update it
		ld (p2Position),A

		end_p2TryUp:
		ret
	
	; check if the player2 can move down ad if it can do it
	p2_tryingMoveDown:
		ld A,(p2Position)
		cmp A,PLAYER_Y_MIN		; 	if not p2Position < minPosition
		jc end_p2TryDown		; 		p2Position -= speed
		sub A,PLAYER_SPEED
		ld (p2Position),A

		end_p2TryDown:
		ret
; ------------------------------




; INCLUDING 'ball.asm':
; ------------------------------
; move the ball according to its current direction and check for collisions to change it
moveBall:

	; this can probably be optimised, but who cares
	ball_move:
	ld A,(ballDir)			;	switch(ballDir)
	cmp A,DIR_TOP_RIGHT		;		case "top right":
	jz ball_move_tr			;			ball_moveTopRight()
	cmp A,DIR_BTM_RIGHT		;		case "bottom right"
	jz ball_move_br			;			ball.moveBottomRight()
	cmp A,DIR_BTM_LEFT		;		case "bottom left"
	jz ball_move_bl			;			ball.moveBottomLeft()
	jp ball_move_tl			;		else => ball.moveTopLeft()

	ball_move_tr:			; top right		=> x increases y increases
		ld A,(ballXPos)
		add A,BALL_SPEED
		ld (ballXPos),A
		ld A,(ballYPos)
		add A,BALL_SPEED
		ld (ballYPos),A
		jp ball_checkForCollisions_tr
	ball_move_br:			; bottom right	=> x increases y decreases
		ld A,(ballXPos)
		add A,BALL_SPEED
		ld (ballXPos),A
		ld A,(ballYPos)
		sub A,BALL_SPEED
		ld (ballYPos),A
		jp ball_checkForCollisions_br
	ball_move_bl:			; bottom left	=> x decreases y decreases
		ld A,(ballXPos)
		sub A,BALL_SPEED
		ld (ballXPos),A
		ld A,(ballYPos)
		sub A,BALL_SPEED
		ld (ballYPos),A
		jp ball_checkForCollisions_bl
	ball_move_tl:			; top left		=> x decreases y increases
		ld A,(ballXPos)
		sub A,BALL_SPEED
		ld (ballXPos),A
		ld A,(ballYPos)
		add A,BALL_SPEED
		ld (ballYPos),A
		jp ball_checkForCollisions_tl

	ball_checkForCollisions_tr:		; since we're moving top-right we'll only check in that direction
		ball_check_tr_y:
			ld A,(ballYPos)
			cmp A,BALL_Y_MAX
			jc ball_check_tr_x		;	if not pos.y < y_max
			ld A,DIR_BTM_RIGHT		;		direction = bottom_right
			ld (ballDir),A
		ball_check_tr_x:
			ld A,(ballXPos)
			cmp A,BALL_P2_CONTACT		;	if not pos.x < p2.pos.x
			jc end_ball_check_tr_x		;	
			ld A,(ballYPos)
			ld B,(p2Position)			;	if not pos.y < p2.pos.y
			cmp A,B						;	
			jc end_ball_check_tr_x
			ld B,A
			ld A,(p2Position)
			add A,PLAYER_HEIGHT			;	if not (p2.pos.y + p2.height) < pos.y
			cmp A,B
			jc end_ball_check_tr_x						

			; if all above are true:
			ld A,DIR_TOP_LEFT
			ld (ballDir),A				; direction = "top left"

		end_ball_check_tr_x:
		jp end_moveBall
	ball_checkForCollisions_br:
		ball_check_br_y:
			ld A,BALL_Y_MIN
			ld B,(ballYPos)
			cmp A,B
			jc ball_check_br_x		;	if pos.y < y_min
			ld A,DIR_TOP_RIGHT		;		direction = "top right"
			ld (ballDir),A
		ball_check_br_x:
			ld A,(ballXPos)
			cmp A,BALL_P2_CONTACT		;	if not pos.x < p2.pos.x
			jc end_ball_check_br_x		;	
			ld A,(ballYPos)
			ld B,(p2Position)			;	if not pos.y < p2.pos.y
			cmp A,B						;	
			jc end_ball_check_br_x
			ld B,A
			ld A,(p2Position)
			add A,PLAYER_HEIGHT			;	if not (p2.pos.y + p2.height) < pos.y
			cmp A,B
			jc end_ball_check_br_x

			; if all above are true:
			ld A,DIR_BTM_LEFT
			ld (ballDir),A				; direction = "bottom left"

		end_ball_check_br_x:
		jp end_moveBall
	ball_checkForCollisions_bl:
		ball_check_bl_y:
			ld A,BALL_Y_MIN
			ld B,(ballYPos)
			cmp A,B
			jc ball_check_bl_x
			ld A,DIR_TOP_LEFT
			ld (ballDir),A
		ball_check_bl_x:
			ld A,BALL_P1_CONTACT
			ld B,(ballXPos)
			cmp A,B
			jc end_ball_check_bl_x
			ld A,(ballYPos)
			ld B,(p1Position)
			cmp A,B
			jc end_ball_check_bl_x
			ld B,A
			ld A,(p1Position)
			add A,PLAYER_HEIGHT
			cmp A,B
			jc end_ball_check_bl_x						

			; if all above are true:
			ld A,DIR_BTM_RIGHT
			ld (ballDir),A

		end_ball_check_bl_x:
		jp end_moveBall
	ball_checkForCollisions_tl:		; since we're moving top-left we'll only check in that direction
		ball_check_tl_y:
			ld A,(ballYPos)
			cmp A,BALL_Y_MAX
			jc ball_check_tl_x
			ld A,DIR_BTM_LEFT
			ld (ballDir),A
		ball_check_tl_x:
			ld A,BALL_P1_CONTACT
			ld B,(ballXPos)
			cmp A,B
			jc end_ball_check_tl_x
			ld A,(ballYPos)
			ld B,(p1Position)
			cmp A,B
			jc end_ball_check_tl_x
			ld B,A
			ld A,(p1Position)
			add A,PLAYER_HEIGHT
			cmp A,B
			jc end_ball_check_tl_x						

			; if all above are true:
			ld A,DIR_TOP_RIGHT
			ld (ballDir),A

		end_ball_check_tl_x:
		jp end_moveBall

	end_moveBall:
	ret
; ------------------------------




; INCLUDING 'scoresAndWon.asm':
; ------------------------------
; called when players score a goal
player1ScoredRoutine:
	call cls

	; update the score
	ld A,(p1Score)
	inc A
	cmp A,SCORE_WIN
	jz player1Won
	ld (p1Score),A

	; change the ball's position
	ld A,BALL_RESTART_POS_P2	; ball will be placed in front of p2
	ld (ballXPos),A
	ld A,BALL_START_YPOS
	ld (ballYPos),A

	; change the ball's direction
	ld A,(p1Score)	; not necessary but at least it won't always start in the same direction this way
	bit 0,A
	jz p1Score_dir_update_1		;
	ld A,DIR_TOP_LEFT			;	if p1Score % 2 == 1
	jp p1Score_dir_update_2		;		ballDirection = "top left"
	p1Score_dir_update_1:		;	else
	ld A,DIR_BTM_LEFT			;		ballDirection = "bottom left"
	p1Score_dir_update_2:		;
	ld (ballDir),A

	ret
player2ScoredRoutine:
	call cls
	
	; update the score
	ld A,(p2Score)
	inc A
	cmp A,SCORE_WIN
	jz player2Won
	ld (p2Score),A

	; change the ball's position
	ld A,BALL_RESTART_POS_P1	; ball will be placed in front of p1
	ld (ballXPos),A
	ld A,BALL_START_YPOS
	ld (ballYPos),A

	; change the ball's direction
	ld A,(p2Score)	; not necessary but at least it won't always start in the same direction this way
	bit 0,A
	jz p2Score_dir_update_1		;
	ld A,DIR_TOP_RIGHT			;	if p2Score % 2 == 1
	jp p2Score_dir_update_2		;		ballDirection = "top right"
	p2Score_dir_update_1:		;	else
	ld A,DIR_BTM_RIGHT			;		ballDirection = "bottom right"
	p2Score_dir_update_2:		;
	ld (ballDir),A

	ret
player1Won:
	ld HL,p_won_msg
	ld AB,HL
	ld HL,PWON_MSG_ADDR

	; copy the entire message
	ccr (HL),(AB)
	ccr (HL),(AB)
	ccr (HL),(AB)
	ccr (HL),(AB)
	ccr (HL),(AB)
	ccr (HL),(AB)

	; now we'll write the player number (one)
	ld HL,image_one
	ld AB,HL
	ld HL,PWON_MSG_ADDR
	add HL,8
	ccr (HL),(AB)

	halt
	stop
player2Won:
	ld HL,p_won_msg
	ld AB,HL
	ld HL,PWON_MSG_ADDR

	; copy the entire message
	ccr (HL),(AB)
	ccr (HL),(AB)
	ccr (HL),(AB)
	ccr (HL),(AB)
	ccr (HL),(AB)
	ccr (HL),(AB)

	; now we'll write the player number (two)
	ld HL,image_two
	ld AB,HL
	ld HL,PWON_MSG_ADDR
	add HL,8
	ccr (HL),(AB)

	halt
	stop
; ------------------------------




; INCLUDING 'div.asm':
; ------------------------------
div:        ; func div(dividend => A, divisor => B) --> (remainder => A, quotient => C)
    ld c,0      ; how many times were we able to subtract

    divloop:
        cmp A,b
        jc enddiv	; if A is smaller than B we are done

        inc c		; else we're able to sub once more
        sub A,b		; so sub
        jp divloop	; and repeat

    enddiv:
    ret
; ------------------------------




; INCLUDING 'graphics.asm':
; ------------------------------
; totally not copied from pokémon RBY
image_zero:
db 0
db 0b0011_1000
db 0b0100_1100
db 0b1100_0110
db 0b1100_0110
db 0b0110_0100
db 0b0011_1000
db 0
image_one:
db 0
db 0b0001_1000
db 0b0011_1000
db 0b0001_1000
db 0b0001_1000
db 0b0001_1000
db 0b0111_1110
db 0
image_two:
db 0
db 0b0111_1100
db 0b1100_0110
db 0b0000_1110
db 0b0111_1000
db 0b1110_0000
db 0b1111_1110
db 0
image_tre:
db 0
db 0b0111_1110
db 0b0000_1100
db 0b0011_1000
db 0b0000_0110
db 0b1100_0110
db 0b0111_1100
db 0
image_four:
db 0
db 0b0001_1100
db 0b0011_1100
db 0b0110_1100
db 0b1100_1100
db 0b1111_1110
db 0b0000_1100
db 0
image_five:
db 0
db 0b1111_1100
db 0b1100_0000
db 0b1111_1100
db 0b0000_0110
db 0b1100_0110
db 0b0111_1100
db 0
image_six:
db 0
db 0b0111_1100
db 0b1100_0000
db 0b1111_1100
db 0b1100_0110
db 0b1100_0110
db 0b0111_1100
db 0
image_seven:
db 0
db 0b1111_1110
db 0b1100_0110
db 0b0000_1100
db 0b0001_1000
db 0b0011_0000
db 0b0011_0000
db 0
image_eight:
db 0
db 0b0111_1100
db 0b1100_0110
db 0b0111_1100
db 0b1100_0110
db 0b1100_0110
db 0b0111_1100
db 0
image_nine:
db 0
db 0b0111_1100
db 0b1100_0110
db 0b1100_0110
db 0b0111_1110
db 0b0000_0110
db 0b0111_1100
db 0

p_won_msg:
; p
db 0b1111_1100
db 0b1000_0010
db 0b1000_0010
db 0b1111_1100
db 0b1000_0000
db 0b1000_0000
db 0b1000_0000
db 0
; space for player number
db 0,0,0,0,0,0,0,0
; space
db 0,0,0,0,0,0,0,0
; w
db 0b1000_0010
db 0b1001_0010
db 0b1010_1010
db 0b1010_1010
db 0b1100_0110
db 0b1100_0110
db 0b1000_0010
db 0
; o
db 0b0011_1000
db 0b0100_0100
db 0b1000_0010
db 0b1000_0010
db 0b1000_0010
db 0b01_000100
db 0b0011_1000
db 0
; n
db 0b1000_0010
db 0b1100_0010
db 0b1010_0010
db 0b1001_0010
db 0b1000_1010
db 0b1000_0110
db 0b1000_0010
db 0
; ------------------------------



; org 0x8000
; variables
ballXPos:
db BALL_RESTART_POS_P1
ballYPos:
db BALL_START_YPOS
ballDir:
db DIR_TOP_RIGHT
p1Position:	; position of the lowest pixel
db 112
p2Position:
db 112
p1Score:
db 0
p2Score:
db 0
; temp variables
tempBallAddrStorage_C:
db 00
tempBallAddrStorage_D:
db 00

