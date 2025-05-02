;
;	Compare TOS with A
;

	.export __ccnec

__ccnec:
	sub a,2,p1
	pop p2		; return
	pop p3		; arg
	bz false	; already 0
	ld ea,=1	;  true
false:
	push p2
	ret
