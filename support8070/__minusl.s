	.export __minusl

	.code

	; Subtract hireg,ea from TOS, drop TOS
__minusl:
	push ea
	ld ea,4,p1	; low half
	sub ea,0,p1	; low half
	st ea,0,p1	; stash back
	ld a,s
	bp skip		; carry : FIXME check carry/borrow
	ld ea,:__hireg
	sub ea,=1
subexit:
	sub ea,6,p1	; high half
	st ea,:__hireg
	pop ea		; result
	pop p2		; return
	pop p3
	pop p3
	push p2
	ret		; and return
skip:
	ld ea,:__hireg
	bra subexit
