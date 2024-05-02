	.export __switchc

	.setcpu 6803

__switchc:
	; X holds the switch table, B the value
	inx
	ldaa ,x		; Table size (will always be < 256 entries)
	inx
	beq gotit
next:
	cmpb ,x
	inx
	inx
	beq gotit
	inx
	inx
	deca
	bne next
gotit:
	ldx ,x
	jmp ,x
