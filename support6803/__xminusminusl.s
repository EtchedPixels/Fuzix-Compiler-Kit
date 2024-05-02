;
;	,X - Y:D, return old ,X
;
	.export __xmminusl
	.export __xmminusul

	.setcpu 6803
	.code

__xmminusl:
__xmminusul:
	std @tmp
	ldd 2,x		; low half
	pshb		; save
	psha
	subd @tmp	; result
	std 2,x
	ldd ,x
	pshb
	psha		; stack the upper half
	sbcb @hireg+1
	sbca @hireg
	std ,x
	pula
	pulb
	std @hireg
	pula
	pulb
	rts
