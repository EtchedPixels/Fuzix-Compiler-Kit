;
;        jsr __loadnl
;        .word address
;
	.export __loadnl
	.code
__loadnl:
	stx @tmp	; X where we can manipulate it
	tsx
	ldx 0,x		; return address
	stx @tmp2	; save return address
	tsx		; get sp again
	ldab @tmp2+1	; calculate new return address
	addb #2
	stab 1,x	; restore new return address
	ldab @tmp2
	adcb #0
	stab 0,x
	ldx @tmp2	; get word address again
	clrb
	stab @hireg+1
	stab @hireg
	ldab 1,x
	ldaa 0,x
	ldx @tmp
	rts
