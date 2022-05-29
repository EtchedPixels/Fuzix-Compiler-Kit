;
;	Signed division. Do this via unsigned
;
		.setcpu 8080
		.export __div
		.export __divde
		.export __rem
		.export __remde

__div:
	xchg
	pop	h
	xthl
	call	__divde
	xchg
	ret

__divde:
	push	b
	mvi	c,0
	call	signfix
	xchg
	call	signfix
	xchg
	;	C tells us if we need to invert

	call	__divdeu
	xchg

	mov	a,c
	rar
	jnc	noinvert
	call	signfix
noinvert:
	pop	b
	ret

__rem:
	xchg
	pop	h
	xthl
__remde:
	push	b
	call	signfix
	xchg
	mvi	c,0
	call	signfix
	xchg
	;	C tells us if we need to invert

	call	__divdeu

	mov	a,c
	rar
	jnc	noinvertr
	call	signfix
noinvertr:
	pop	b
	ret

;	Turn HL positive, xor a with one if was
signfix:
	mov	a,h
	ora	a
	rp
	cma
	mov	h,a
	mov	a,l
	cma
	mov	l,a
	inx	h
	inr	c
	ret
