	.export __switch

	.setcpu 6803

__switch:
	; X holds the switch table, D the value
	std @tmp
	ldd ,x
	std @tmp1
	inx
	inx
	beq gotit
next:
	ldd ,x
	subd @tmp
	inx
	inx
	beq gotit
	inx
	inx
	dec @tmp1
	bne next
gotit:
	ldx ,x
	jmp ,x
