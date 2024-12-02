;
;	logic ops between hireg:D and TOS
;
	.export __xorl
	.code

	.setcpu 6800

__xorl:
	tsx
	eorb 5,x
	eora 4,x
	stab @tmp+1
	ldx 0,x
	ins
	ins
	pulb
	eorb @hireg
	stab @hireg
	pulb
	eorb @hireg+1
	stab @hireg+1
	ldab @tmp+1
	ins
	ins
	jmp ,x
