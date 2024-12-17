;
;	Compare TOS with EA
;

	.export __ccne

__ccne:
	sub ea,2,p1
	or a,e
	pop p2		; return
	pop p3		; arg
	bz false	; already 0
	ld ea,=1	;  true
false:
	push p2
	ret
