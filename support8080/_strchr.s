;
;	String scan for C
;
		.export _strchr

_strchr:
	lxi	h,2
	dad	sp
	mov	e,m
	inx	h
	mov	d,m
	inx	h
	mov	l,m	; symbol we want
	xchg

next:
	mov	a,m
	ora	a
	jr	z, end
	cmp	e
	rz		; HL points to match
	inx	h
	jmp	next
end:
	lxi	h,0
	ret

	