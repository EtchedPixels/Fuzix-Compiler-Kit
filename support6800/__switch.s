	.export __switch

__switch:
	; X holds the switch table, D the value
	pshb
	ldab 1,x
	inx
	inx
	stab @tmp1+1
	pulb
	beq gotit
next:
	cmpa ,x
	bne	nomatch
	cmpb 1,x
	bne nomatch
	ldx 2,x
	jmp ,x
nomatch:
	inx
	inx
	inx
	inx
	dec @tmp1+1		; We know < 256 entries per switch
	bne	next
gotit:
	ldx ,x
	jmp ,x
