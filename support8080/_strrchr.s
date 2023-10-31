;
;	String scan for C
;
		.export _strrchr

_strrchr:
	push	b
	lxi	h,4
	dad	sp
	mov	e,m
	inx	h
	mov	d,m
	inx	h
	mov	l,m	; symbol we want
	xchg
	lxi	b,0	; Return NULL by default

next:
	mov	a,m
	ora	a
	jr	z, end
	cmp	e
	jnz	nomatch
	; Save the match
	mov	b,h
	mov	c,l
	; Keep scanning as we want the right most
nomatch:
	inx	h
	jmp	next
end:
	mov	l,c
	mov	h,b
	pop	b
	ret
