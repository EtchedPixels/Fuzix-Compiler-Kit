;
;	logic ops between hireg:D and TOS
;
	.export __orl
	.code

	.setcpu 6803

__orl:
	tsx
	orab 0,x
	oraa 1,x
	std @tmp
	ldd @hireg
	orab 2,x
	oraa 3,x
	std @hireg
	ldd @tmp
	pulx
	ins
	ins
	ins
	ins
	jmp ,x
