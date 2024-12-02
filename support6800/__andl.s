;
;	logic ops between hireg:D and TOS
;
	.export __bandl
	.code

	.setcpu 6800

__bandl:
	tsx
	andb 5,x
	anda 4,x
	stab @tmp+1
	ldx 0,x
	ins
	ins
	pulb
	andb @hireg
	stab @hireg
	pulb
	andb @hireg+1
	stab @hireg+1
	ldab @tmp+1
	ins
	ins
	jmp ,x
