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
	inx
	inx
	bne nomat
	cmpa ,x
	bne nomat
	cmpb 1,x
	beq gotit
nomat:
	inx
	inx
incmv:
	inx
	inx
moveon:
	dec @tmp		; We know < 256 entries per switch
	bne next
	bra def
gotit:
	inx
	inx
def:
	ldx ,x
	jmp ,x
