;
;	Switch. We use the tables as is, nothing clever like
;	binary searching yet
;
			.export __switchc
			.export __switchcu
			.setcpu 8080
			.code

__switchcu:
__switchc:
		mov	a,l
		; DE points to the table in the format
		; Length
		; value, label
		; default label
		xchg
		mov	e,m
		inx	h
		mov	d,m
next:
		inx	h		; Move on to value to check
		cmp	m
		inx	h		; Move on to address
		jz	match
		inx	h		; Skip address low
		dcr	e
		jnz	next
		dcr	d
		jnz	next
		; We are pointing at the address
match:
		mov	e,m
		inx	h
		mov	d,m
		xchg
		pchl
