;
;	logic ops between hireg:D and TOS
;
	.export __bandl
	.code

	.setcpu 6803

__bandl:
	tsx
	andb 5,x	; low word
	anda 4,x
	std @tmp	; save
	ldd @hireg
	andb 3,x
	anda 2,x	; high word
	std @hireg
	ldd @tmp	; restore
	pulx		; clean up
	ins
	ins
	ins
	ins
	jmp ,x
