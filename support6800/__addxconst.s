;
;	Add the constant following the call to X without
;	messing up D	
;
	.export __addxconst
	.code

__addxconst:
	staa @tmp2	; Save D
	stab @tmp2+1
	stx @tmp	; X where we can manipulate it
	tsx
	ldx ,x		; Return address
	ins
	ins
	ldaa ,x		; value to add
	ldab 1,x
	inx		; Move to word after
	inx
	stx @tmp1	; save the return address
	addb @tmp+1	; D now holds the right value
	adca @tmp
	staa @tmp	; D into X
	stab @tmp+1
	ldx @tmp
	ldaa @tmp1	; Recover return address
	ldab @tmp1+1
	pshb
	psha
	ldaa @tmp2	; Restore old D
	ldab @tmp2+1
	rts
