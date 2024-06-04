	.export __switch

__switch:
	; X holds the switch table, D the value
	; We can afford to trash Y
	ldy ,x		; Table size
	beq gotit
	inx
	inx
next:
	cmpa ,x
	bne n1
	cmpb 1,x
	bne n1
gotit:
	ldx 2,x
	jmp ,x
n1:
	inx
	inx
	inx
	inx
	dey
	bne next
	ldx ,x
	jmp ,x
