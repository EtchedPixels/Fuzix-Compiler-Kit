;
	.export __addsp8
	.export __subsp8
	.export __modsp16
	.code

__addsp8:
	stab @tmp2+1
	stx @tmp
	tsx
	ldx 0,x
	ins
	ins
	sts @tmp3
	ldab @tmp3+1
	addb 0,x
	stab @tmp3+1
	bcc	__addsp8_1
	inc @tmp3
__addsp8_1:
	inx
	stx @tmp1	; return address
	ldx @tmp
	lds @tmp3
	ldab @tmp1+1
	pshb
	ldab @tmp1
	pshb
	ldab @tmp2+1
	rts

__subsp8:
	stab @tmp2+1
	stx @tmp
	tsx
	ldx 0,x
	ins
	ins
	sts @tmp3
	ldab @tmp3+1
	subb 0,x
	stab @tmp3+1
	bcc	__addsp8_1
	dec @tmp3
	bra	__addsp8_1

__modsp16:
	stab @tmp2+1
	staa @tmp2
	stx @tmp
	tsx
	ldx 0,x
	ins
	ins
	sts @tmp3
	ldab @tmp3+1
	ldaa @tmp3
	addb 1,x
	adca 0,x
	stab @tmp3+1
	staa @tmp3
	inx
	inx
	stx @tmp1	; return address
	ldx @tmp
	lds @tmp3
	ldab @tmp1+1
	ldaa @tmp1
	pshb
	psha
	ldab @tmp2+1
	ldaa @tmp2
	rts
