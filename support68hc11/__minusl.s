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
	; Stack is now
	; 0 - high of value to subtract
	; 2 - low of value to subtract
	; 4 - return address
	; 6 - high word of base value
	; 8 - low word of base value
	tsx
	ldd 8,x		; low
	subd 2,x	; - low
	xgdy
	ldd 6,x
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
