;
;	Until we sort out optimizing this in the compiler proper
;
	.export __minusl
	.code

__minusl:
	; Subtract Y:D from TOS
	pshb
	psha
	pshy
	tsx
	ldd 6,x		; low
	subd 2,x	; - low
	xgdy
	ldd 4,x
	sbcb 1,x
	sbca 0,x
	xgdy
	pulx		; discard pushed work value
	pulx
	pulx		; return
	ins		; discard argument
	ins
	ins
	ins
	rts
