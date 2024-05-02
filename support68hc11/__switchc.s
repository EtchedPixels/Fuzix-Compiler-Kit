	.export __switchc

__switchc:
	; X holds the switch table, D the value
	; We can afford to trash Y
	ldy ,x		; Table size
	inx
	inx
	beq gotit
next:
	cmpb ,x
	inx
	inx
	beq gotit
	inx
	inx
	dey
	bne next
gotit:
	ldx ,x
	jmp ,x
