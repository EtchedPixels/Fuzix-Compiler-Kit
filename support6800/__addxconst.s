;
;	Add the constant following the call to X without
;	messing up D	
;
	.export __addxconst
	.setcpu 6803
	.code

__addxconst:
	std @tmp2	; Save D
	stx @tmp	; X where we can manipulate it
	pulx		; Return address
	ldd ,x		; value to add
	inx		; Move to word after
	inx
	pshx		; Restack return address
	addd @tmp	; D now holds the right value
	std @tmp	; D into X
	ldx @tmp
	ldd @tmp2	; Restore old D
	rts
