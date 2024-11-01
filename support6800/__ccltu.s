;
;	Compare D with TOS
;	TOS < D
;
	.export __ccltu

__ccltu:
    tsx
    suba    2,x
    bhi     true
    bne     false
	subb	3,x
	bhi	true
false:
	jmp	__false2
true:
	jmp	__true2
