;
;	Compare TOS with EA
;

	.export __cceq

__cceq:
	sub ea,2,p1
	or a,e
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
