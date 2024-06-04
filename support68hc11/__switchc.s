	.export __switchc

__switchc:
	; X holds the switch table, B the value
	; We can afford to trash Y
	ldy ,x		; Table size
	beq gotit
	inx
	inx
next:
	cmpb ,x
	beq gotit
	inx
	inx
	inx
	dey
	bne next
	ldx ,x
	jmp ,x
gotit:
	ldx 1,x
	jmp ,x
