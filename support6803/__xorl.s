;
;	logic ops between hireg:D and TOS
;
	.export __xorl
	.code

	.setcpu 6803

__xorl:
	tsx
	eorb 5,x
	eora 4,x
	std @tmp
	ldd @hireg
	eorb 3,x
	eora 2,x
	std @hireg
	ldd @tmp
	pulx
	ins
	ins
	ins
	ins
	jmp ,x
