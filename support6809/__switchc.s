	.export __switchc

__switchc:
	; X holds the switch table, B the value
	; We can afford to trash Y
	ldy ,x++		; Table size
	beq gotit
next:
	cmpb ,x+
	beq gotit
	leax 2,x
	leay -1,y
	bne next
gotit:
	ldx ,x
	jmp ,x
