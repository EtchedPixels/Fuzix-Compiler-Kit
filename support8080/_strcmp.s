;
;	String compare for C
;
		.export _strcmp

_strcmp:
	lxi	h,2
	dad	sp
	mov	e,m
	inx	h
	mov	d,m
	inx	h
	mov	a,m
	inx	h
	mov	h,m
	mov	l,a

next:
	ldax	d
	cmp	m
	inx	h
	inx	d
	jnz	diff
	;	Symbols the same - might both be end marker
	ora	a
	jnz	next
	lxi	h,0
	ret

diff:
	; Symbols differ. End will correctly sort
	;	C means (HL) > (DE) - ie string 1 is bigger
	jc	ret1
	lxi	h,-1
	ret
ret1:
	lxi	h,1
	ret
