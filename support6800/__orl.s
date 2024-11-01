;
;	logic ops between hireg:D and TOS
;
	.export __orl
	.code

	.setcpu 6800

__orl:
	tsx
	orab 5,x
	oraa 4,x
	stab @tmp+1
	staa @tmp
	ldab @hireg+1
	ldaa @hireg
	orab 3,x
	oraa 2,x
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
