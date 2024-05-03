	.export __negatel
	.code

	; The compiler has internal knowledge that this does not affect X
__negatel:
	staa @tmp
	stab @tmp+1
	ldaa @hireg
	ldab @hireg+1
	coma
	comb
	staa @hireg
	stab @hireg+1
	ldaa @tmp
	ldab @tmp+1
	coma
	comb
	addb #1
	adca #0
	bcc nocarry
	inc @hireg+1
	bne nocarry		; inc doesn't touch carry so check for 0
	inc @hireg
nocarry:
	rts

	