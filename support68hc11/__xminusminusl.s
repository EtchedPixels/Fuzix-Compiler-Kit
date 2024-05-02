;
;	,X - Y:D, return old ,X
;
	.export __xmminusl
	.export __xmminusul

	.code
__xmminusl:
__xmminusul:
	std @tmp
	ldd 2,x		; low half
	pshb		; save
	psha
	subd @tmp	; result
	xgdy
	std @tmp
	ldd ,x
	pshb
	psha		; stack the upper half
	sbcb @tmp+1
	sbca @tmp
	std ,x
	puly
	pula
	pulb
	rts
