;
	.export __addsp8
	.export __subsp8
	.export __modsp16
	.code

	.setcpu 6803

__addsp8:
	stab @tmp2+1
	stx @tmp
	pulx
	sts @tmp3
	ldab @tmp3+1
	addb 0,x
	stab @tmp3+1
	bcc	__addsp8_1
	inc @tmp3
__addsp8_1:
	inx
	pshx
	ldx @tmp
	lds @tmp3
	ldab @tmp2+1
	rts

__subsp8:
	stab @tmp2+1
	stx @tmp
	pulx
	sts @tmp3
	ldab @tmp3+1
	subb 0,x
	stab @tmp3+1
	bcc	__addsp8_1
	dec @tmp3
	bra	__addsp8_1

__modsp16:
	std @tmp2
	stx @tmp
	pulx
	sts @tmp3
	ldd @tmp3
	addb 1,x
	adca 0,x
	stab @tmp3+1
	staa @tmp3
	inx
	inx
	pshx
	ldx @tmp
	lds @tmp3
	ldd @tmp2
	rts
