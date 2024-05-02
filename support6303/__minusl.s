;
;	Until we sort out optimizing this in the compiler proper
;
	.export __minusl
	.code

	.setcpu 6803

__minusl:
	; Subtract hireg:D from TOS
	tsx
	std @tmp
	ldd 4,x
	subd @tmp
	std @tmp
	ldd 2,x
	sbcb @hireg+1
	sbca @hireg
	std @hireg
	ldd @tmp
	rts
