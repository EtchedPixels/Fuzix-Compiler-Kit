	.export __switchl

__switchl:
	; X holds the switch table, hireg:D the value
	; Juggle as we are short of regs here - TODO find a nicer approach
	staa @tmp
	stab @tmp+1
	ldab 1,x
	inx
	inx
	tstb
	beq gotit
next:
	ldaa ,x
	cmpa @hireg
	bne nomat
	ldaa 1,x
	cmpa @hireg+1
	bne nomat
	ldaa 2,x
	cmpa @tmp
	bne nomat
	ldaa 3,x
	cmpa @tmp+1
	bne	nomat
	ldx	4,x
	jmp	,x
nomat:
	inx
	inx
	inx
	inx
	inx
	inx
moveon:
	decb			; We know < 256 entries per switch
	bne next
gotit:
	ldx ,x
	jmp ,x
