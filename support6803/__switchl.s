	.export __switchl

	.setcpu 6803

__switchl:
	; X holds the switch table, hireg:D the value
	; Juggle as we are short of regs here - TODO find a nicer approach
	std @tmp
	ldab ,x
	stab @tmp1
	inc @tmp1
	bra incmv
next:
	ldd ,x
	subd @hireg
	inx
	inx
	bne nomat
	ldd @tmp
	subd ,x
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
