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
	ldx 0,x
	ins
	ins
	pulb
	orab @hireg
	stab @hireg
	pulb
	orab @hireg+1
	stab @hireg+1
	ldab @tmp+1
	ins
	ins
	jmp ,x
