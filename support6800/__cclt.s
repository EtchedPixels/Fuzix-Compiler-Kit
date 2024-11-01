;
;	Compare D with TOS
;	TOS < D
;
	.export __cclt

__cclt:
    tsx
    suba    2,x
    bgt     true
    bne     false
	subb	3,x
	bhi	true
false:
	jmp	__false2
true:
	jmp	__true2
