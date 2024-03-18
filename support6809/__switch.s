	.export __switch

__switch:
	; X holds the switch table, D the value
	; We can afford to trash Y
	ldy ,x++		; Table size
	beq gotit
next:
	cmpd ,x++
	beq gotit
	leax 2,x
	leay -1,y
	bne next
gotit:
	ldx ,x
	jmp ,x
