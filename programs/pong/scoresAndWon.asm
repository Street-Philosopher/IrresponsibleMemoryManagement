; called when players score a goal
player1ScoredRoutine:
	call cls

	; update the score
	ld HL,image_one
	ld A,(p1Score)			;
	inc A					; p1Score++;
	cmp A,SCORE_WIN			; if p1Score == winScore
	call z,winRoutine		;		WinRoutine(player1)
	ld (p1Score),A			;

	; change the ball's position
	ld A,BALL_RESTART_POS_P2	; ball will be placed in front of p2
	ld (ballXPos),A
	ld A,BALL_START_YPOS
	ld (ballYPos),A

	; change the ball's direction
	ld A,(p1Score)	; not necessary but at least it won't always start in the same direction this way
	bit 0,A
	jr z,p1Score_dir_update_1	;
	ld A,DIR_TOP_LEFT			;	if p1Score % 2 == 1
	jr p1Score_dir_update_2		;		ballDirection = "top left"
	p1Score_dir_update_1:		;	else
	ld A,DIR_BTM_LEFT			;		ballDirection = "bottom left"
	p1Score_dir_update_2:		;
	ld (ballDir),A

	ret
player2ScoredRoutine:
	call cls
	
	; update the score
	ld HL,image_two
	ld A,(p2Score)			;
	inc A					; p2Score++;
	cmp A,SCORE_WIN			; if p2Score == winScore
	call z,winRoutine		;		WinRoutine(player2)
	ld (p2Score),A			;

	; change the ball's position
	ld A,BALL_RESTART_POS_P1	; ball will be placed in front of p1
	ld (ballXPos),A
	ld A,BALL_START_YPOS
	ld (ballYPos),A

	; change the ball's direction
	ld A,(p2Score)	; not necessary but at least it won't always start in the same direction this way
	bit 0,A
	jr z,p2Score_dir_update_1		;
	ld A,DIR_TOP_RIGHT			;	if p2Score % 2 == 1
	jr p2Score_dir_update_2		;		ballDirection = "top right"
	p2Score_dir_update_1:		;	else
	ld A,DIR_BTM_RIGHT			;		ballDirection = "bottom right"
	p2Score_dir_update_2:		;
	ld (ballDir),A

	ret

; parameters:
;		HL = address to the image of the player that won
winRoutine:
	; first we'll write the player number
	ld AB,HL
	ld HL,PWON_MSG_ADDR
	add HL,8
	ccr (HL),(AB)

	; now write the rest of the message
	ld HL,p_won_msg
	ld AB,HL
	ld HL,PWON_MSG_ADDR

	; copy the entire message
	ccr (HL),(AB)
	add HL,8		; leave a space for the number we already wrote
	ccr (HL),(AB)
	ccr (HL),(AB)
	ccr (HL),(AB)
	ccr (HL),(AB)

	halt
	stop
