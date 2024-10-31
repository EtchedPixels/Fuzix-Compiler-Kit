;
;
	.export __lplus
	.export __lplusb
	.export __lplus2
	.export __lplusb2
	.export __lplusxb
	.code

;	The common forms for array scaling moved by peephole
__lplusb2:
	lslb
	rola
;	D = D + byte + sp
__lplusb:
	stx	@tmp
	tsx
	ldx 0,x
	addb 0,x		; value to add
	adca #0
	bra	__lplus_s
__lplus2:
	lslb
	rola
;	D = D + word + sp
__lplus:
	stx	@tmp
	tsx
	ldx 0,x
	addb 1,x		; value to add
	adca 0,x
	inx
__lplus_s:
	inx
	stx @tmp1		; save the return address
	ins
	ins
	sts @tmp2
	addb @tmp2+1	; D now holds the right value
	adca @tmp2
	stab @tmp2+1
	ldx @tmp
__lplus_e:
	ldab @tmp1+1	; Recover return address
	pshb
	ldab @tmp1
	pshb
	ldab @tmp2+1
	rts

;	D and X = D + byte + sp
__lplusxb:
	tsx
	ldx 0,x
	addb 0,x		; value to add
	adca #0
	bra	__lplusx_s
__lplusx:
	tsx
	ldx 0,x
	addb 1,x		; value to add
	adca 0,x
	inx
__lplusx_s:
	inx
	stx @tmp1		; save the return address
	ins
	ins
	sts @tmp2
	addb @tmp2+1	; D now holds the right value
	adca @tmp2
	stab @tmp2+1
	staa @tmp2
	ldx @tmp2
	bra	__lplus_e
