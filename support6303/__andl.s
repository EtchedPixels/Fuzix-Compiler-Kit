;
;	logic ops between hireg:D and TOS
;
	.export __bandl
	.code

	.setcpu 6803

__bandl:
	tsx
	andb 5,x
	anda 4,x
	std @tmp
	ldd @hireg
	andb 3,x
	anda 2,x
	std @hireg
	ldd @tmp
	pulx
	ins
	ins
	ins
	ins
	jmp ,x
