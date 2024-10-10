	.export __switchc

__switchc:
	; X holds the switch table, B the value
	ldaa 1,x		; Table size (will always be < 256 entries)
	inx
	inx
	tsta
	beq gotit
next:
	cmpb 0,x
	bne next2
	inx
	bra	gotit
next2:
	inx
	inx
	inx
	deca
	bne next
gotit:
	ldx ,x
	jmp ,x
