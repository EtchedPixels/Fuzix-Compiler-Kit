;
;	logic ops between hireg:D and TOS
;
	.export __orl
	.code

	.setcpu 6803

__orl:
	tsx
	orab 5,x
	oraa 4,x
	std @tmp
	ldd @hireg
	orab 3,x
	oraa 2,x
	std @hireg
	ldd @tmp
	pulx
	ins
	ins
	ins
	ins
	jmp ,x
