	.export __switch

__switch:
	; X holds the switch table, D the value
	staa @tmp
	stab @tmp+1
	ldaa ,x
	ldab 1,x
	staa @tmp1
	stab @tmp1+1
	inx
	inx
	beq gotit
next:
	ldaa ,x
	ldab 1,x
	cmpa @tmp
	bne nomatch
	cmpb @tmp+1
nomatch:
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
