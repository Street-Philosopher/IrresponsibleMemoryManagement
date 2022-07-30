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