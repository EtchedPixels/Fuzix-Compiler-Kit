;
;	Messy need to think how to do it better
;
	.export __xpluseql
	.export __xplusequl

	.code

__xpluseql:
__xplusequl:
	; add hireg:D to ,X
	addb 3,x
	adca 2,x
	staa 2,x
	stab 3,x
	ldaa @hireg+1
	adca 1,x
	staa 1,x
	staa @hireg+1
	ldaa @hireg
	adca ,x
	staa ,x
	staa @hireg
	ldaa 2,x
	rts
