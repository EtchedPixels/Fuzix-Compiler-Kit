;
;
	.export __local_b
	.export __local_d
	.code

__local_b_e:
	stx	@tmp		; save X (needed?)
	tsx
	ldx 2,x
	rts
__local_d:
	bsr __local_b_e
	ldaa 0,x		; value to add
	inx
	bra	__local_b_s
__local_b:
	bsr __local_b_e
	clra
__local_b_s:
	ldab 0,x		; value to add
	inx
	stx @tmp1		; save the return address
	ins
	ins
	sts @tmp2
	addb @tmp2+1	; D now holds the right value
	adca @tmp2
	stab @tmp2+1
	ldab @tmp1+1	; Recover return address
	pshb
	ldab @tmp1
	pshb
	ldab @tmp2+1
	ldx @tmp
	rts
