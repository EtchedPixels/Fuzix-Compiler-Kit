	.export __switchl

__switchl:
	; X holds the switch table, Y:D the value
	; Juggle as we are short of regs here - TODO find a nicer approach
	pshy
	ldy ,x
	sty @tmp
	puly
	inc @tmp
	bra incmv
next:
	cpy ,x
	bne nomat
	cmpa 2,x
	bne nomat
	cmpb 3,x
	bne nomat
	ldx 4,x
	jmp ,x
nomat:
	inx
	inx
	inx
	inx
incmv:
	inx
	inx
moveon:
	dec @tmp		; We know < 256 entries per switch
	bne next
	ldx ,x
	jmp ,x
