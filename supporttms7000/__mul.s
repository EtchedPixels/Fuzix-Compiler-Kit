	.export __mulu
	.export __mul

	.code

;
;	 TOS x ac
;
__mulu:
__mul:
	call @__pop10
	mpy r5,r11
	movd b,r3		; Save low result

	mpy r5,r10
	add b,r2

	mpy r4,r11
	add b,r2

	movd r3,r5
	rets

