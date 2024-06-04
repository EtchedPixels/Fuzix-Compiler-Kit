;
;	logic ops between hireg:D and TOS
;
	.export __xorl
	.code

	.setcpu 6803

__xorl:
	tsx
	eorb 0,x
	eora 1,x
	std @tmp
	ldd @hireg
	eorb 2,x
	eora 3,x
	std @hireg
	ldd @tmp
	pulx
	ins
	ins
	ins
	ins
	jmp ,x
