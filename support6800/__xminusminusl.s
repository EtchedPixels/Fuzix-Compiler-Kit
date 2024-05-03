;
;	,X - Y:D, return old ,X
;
	.export __xmminusl
	.export __xmminusul

	.code

__xmminusl:
__xmminusul:
	staa @tmp
	stab @tmp+1
	ldaa 2,x	; low half
	ldab 3,x
	pshb		; save
	psha
	subb @tmp+1
	sbca @tmp	; result
	staa 2,x
	stab 3,x
	ldaa ,x
	ldab 1,x
	pshb
	psha		; stack the upper half
	sbcb @hireg+1
	sbca @hireg
	staa ,x
	stab 1,x
	pula
	pulb
	staa @hireg
	stab @hireg+1
	pula
	pulb
	rts
