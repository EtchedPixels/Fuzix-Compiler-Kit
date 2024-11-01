;
;	Until we sort out optimizing this in the compiler proper
;
	.export __minusl
	.code

__minusl:
	; Subtract hireg:D from TOS
	tsx
	staa @tmp
	stab @tmp+1
	ldaa 4,x
	ldab 5,x
	subb @tmp+1
	sbca @tmp
	staa @tmp
	stab @tmp+1
	ldaa 2,x
	ldab 3,x
	sbcb @hireg+1
	sbca @hireg
	staa @hireg
	stab @hireg+1
	ldaa @tmp
	ldab @tmp+1
	tsx
	ldx	0,x
	ins
	ins
	ins
	ins
	ins
	ins
	jmp ,x
