;
;	logic ops between hireg:D and TOS
;
	.export __bandl
	.code

	.setcpu 6803

__bandl:
	tsx
	andb 0,x
	anda 1,x
	std @tmp
	ldd @hireg
	andb 2,x
	anda 3,x
	std @hireg
	ldd @tmp
	pulx
	ins
	ins
	ins
	ins
	jmp ,x
