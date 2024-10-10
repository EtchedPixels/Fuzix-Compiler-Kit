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
	staa @tmp
	ldab @hireg+1
	ldaa @hireg
	eorb 3,x
	eora 2,x
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
