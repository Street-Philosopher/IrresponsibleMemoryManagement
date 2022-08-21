; my respect for people who coded entire games in assembly is immeasurable



#define PLAYER_HEIGHT as 48

#define PLAYER_Y_MAX as 206
#define PLAYER_Y_MIN as 16

#define PLAYER_SPEED as 8			; pixels per cycle

#define BALL_P1_CONTACT as  24
#define BALL_P2_CONTACT as 232
#define BALL_Y_MAX as 240
#define BALL_Y_MIN as  16
#define SCORED_TO_P1 as  16
#define SCORED_TO_P2 as 240
#define BALL_START_YPOS as 124
#define BALL_RESTART_POS_P1 as 24
#define BALL_RESTART_POS_P2 as 232

#define BALL_SPEED as 8			; pixels per cycle
#define DIR_TOP_RIGHT as 0
#define DIR_BTM_RIGHT as 1
#define DIR_BTM_LEFT as 2
#define DIR_TOP_LEFT as 3

#define FULL_BLACK as 0x00
#define FULL_WHITE as 0xFF

; values for movement of the pads
#define SPEED_NO as 0
#define SPEED_UP as 1
#define SPEED_DOWN as 2

#define CRAM_INPUT as 0x80

#define FULL_BLACK as 0x00
#define FULL_WHITE as 0xff

#define P1_SCORE_ADDR as 0x0168
#define P2_SCORE_ADDR as 0x0188
#define PWON_MSG_ADDR as 0x0870

#define SCORE_WIN as 2


init:
	; bad and can be optimised easily but who cares, it's more readable
	ld a,  BALL_RESTART_POS_P1
	ld hl, ballXPos
	ld [hl], a
	ld a,  BALL_START_YPOS
	ld hl, ballYPos
	ld [hl], a
	ld a,  DIR_TOP_RIGHT
	ld hl, ballDir
	ld [hl], a
	ld a,  112
	ld hl, p1Position
	ld [hl], a
	ld a,  112
	ld hl, p2Position
	ld [hl], a
	ld a,  0
	ld hl, p1Score
	ld [hl], a
	ld a,  0
	ld hl, p2Score
	ld [hl], a


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

#include "drawing.asm"

#include "players.asm"

#include "ball.asm"

#include "scoresAndWon.asm"

#include "div.asm"

#include "graphics.asm"

; variables
padto 0x8000
ballXPos:
pad
ballYPos:
pad
ballDir:
pad
p1Position:	; position of the lowest pixel
pad
p2Position:
pad
p1Score:
pad
p2Score:
pad
; temp variables
tempBallAddrStorage_C:
pad
tempBallAddrStorage_D:
pad

