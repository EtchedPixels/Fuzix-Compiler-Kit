;
;	Compare TOS with A
;

	.export __cceqc

__cceqc:
	sub a,2,p1
	pop p2		; return
	pop p3		; arg
	bz true
	ld ea,=0
	push p2
	ret
true:
	ld ea,=1
	push p2
	ret
