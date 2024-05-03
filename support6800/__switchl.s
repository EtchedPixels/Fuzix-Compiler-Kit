	.export __switchl

__switchl:
	; X holds the switch table, hireg:D the value
	; Juggle as we are short of regs here - TODO find a nicer approach
	staa @tmp
	stab @tmp+1
	ldab ,x
	stab @tmp1
	inc @tmp1
	bra incmv
next:
	ldaa ,x
	cmpa @hireg
	bne nom
	ldab 1,x
	cmpb @hireg+1
nom:
	inx
	inx
	bne nomat
	ldaa ,x
	cmpa @tmp
	bne nomat
	ldab 1,x
	cmpb @tmp+1
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
