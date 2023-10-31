;
;	String cat for C
;
		.export _strlcat:

_strlcat:
	push	b	; save register variable BC
	lxi	h,9
	dad	sp
	mov	c,m
	dex	h
	mov	b,m
	dex	h
	mov	e,m
	dex	h
	mov	d,m
	dex	h
	mov	a,m
	dex	h
	mov	l,m
	mov	h,a

	; copy from DE to HL limit length BC
	push	b	; limit size - needed for return value
find:
	mov	a,m
	ora	a
	jz	copy
	inx	h
	dex	b
	mov	a,b
	ora	c
	jnz	find
	pop	h	; All space used so return length given
	pop	b	; Recover register variable
	ret
copy:
	ldax	d
	mov	m,a
	inx	h
	inx	d
	ora	a
	jz	done
	dex	b
	mov	a,b
	ora	c
	jnz	copy
done:
	; Hit the end
	pop	h
	mov	a,l
	sub	c
	mov	l,a
	mov	a,h
	sbc	b
	mov	h,a
	pop	b	; and recover register variable BC
	ret
	