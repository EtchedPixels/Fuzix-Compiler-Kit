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
	staa @tmp
	ldab @hireg+1
	ldaa @hireg
	andb 3,x
	anda 2,x
	stab @hireg+1
	staa @hireg
	ldab @tmp+1
	ldaa @tmp
	tsx
	ldx	0,x
	ins
	ins
	ins
	ins
	ins
	ins
	jmp ,x
