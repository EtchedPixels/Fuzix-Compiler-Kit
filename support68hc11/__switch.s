	.export __switch

__switch:
	; X holds the switch table, D the value
	; We can afford to trash Y
	ldy ,x		; Table size
	inx
	inx
	beq gotit
next:
	cmpa ,x
	bne n1
	cmpb 1,x
n1:
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
