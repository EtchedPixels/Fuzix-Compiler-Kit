	.export __switch

	.setcpu 6803

__switch:
	; X holds the switch table, D the value
	std @tmp		; comparison value
	ldd ,x
	stab @tmp1		; get length (will be < 256)
	beq gotit		; zero length table -> take default
	inx
	inx
next:
	ldd ,x			; get arg
	subd @tmp		; compare
	beq gotit
	inx			; skip to next record
	inx
	inx
	inx
	dec @tmp1
	bne next
	ldx ,x			; take default
	jmp ,x
gotit:
	ldx 2,x
	jmp ,x
