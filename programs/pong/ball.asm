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
