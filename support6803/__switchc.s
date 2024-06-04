	.export __switchc

	.setcpu 6803

__switchc:
	; X holds the switch table, B the value
	inx
	ldaa ,x		; Table size (will always be < 256 entries)
	beq gotit
	inx
next:
	cmpb ,x
	beq gotit
	inx
	inx
	inx
	deca
	bne next
	ldx ,x
	jmp ,x
gotit:
	ldx 1,x
	jmp ,x
