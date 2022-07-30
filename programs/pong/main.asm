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


#include drawing.asm

#include players.asm

#include ball.asm

#include scoresAndWon.asm

#include div.asm

#include graphics.asm


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

