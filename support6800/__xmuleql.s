
	.export __xmuleql
	.export __xmulequl

__xmuleql:
__xmulequl:
	; Arg in hireg:D ptr in X
	stx @tmp4
	pshb
	psha
	ldaa @hireg
	ldab @hireg+1
	pshb
	psha
	ldaa ,x
	ldab 1,x
	staa @hireg
	stab @hireg+1
	ldaa 2,x
	ldab 3,x

	; Now we can use the standardc _mull helper

	jsr __mull

	ldx @tmp4
	; Result is in hireg:D, argument was cleared by called fn
	staa 2,x
	stab 3,x
	ldaa @hireg
	staa ,x
	ldaa @hireg+1
	staa 1,x
	ldaa 2,x
	rts
