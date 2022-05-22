		.export __shrequc
		.export __shreqc
		.setcpu 8080
		.code

__shreqc:
	xchg
	pop	h
	xthl

	mov	a,m
	ora	a
	jz	done
	jp	plus
nextm:
	stc
	rar
	dcr	e
	jnz	nextm
done:
	mov	l,a
	ret

__shrequc:
	xchg
	pop	h
	xthl
plus:
	ora	a
	rar
	dcr	e
	jnz	plus
	mov	l,a
	ret
