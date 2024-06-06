	.export __switchl

	.setcpu 6803

__switchl:
	; X holds the switch table, hireg:D the value
	std @tmp
	ldab 1,x		; length
	stab @tmp1
	inc @tmp1
	bra incmv
next:
	ldd ,x
	subd @hireg
	bne nomat
	ldd 2,x
	subd @tmp
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
	dec @tmp1		; We know < 256 entries per switch
	bne next
	ldx ,x
	jmp ,x
