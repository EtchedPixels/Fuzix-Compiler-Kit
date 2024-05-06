;
;	(TOS) *= hireg:B
;
	.setcpu	4
	.export __muleql

	.code

__muleql:
	stx	(-s)
	ldx	2(s)
	; Copy the variable onto the stack
	lda	2(x)
	sta	(-s)
	lda	(x)
	sta	(-s)
	; hireg:B has not been disturbed
	jsr	__mull
	; Result in hireg:B, stack adjusted by called func
	; X was trashed
	ldx	2(s)
	lda	(__hireg)
	sta	(x)
	stb	2(x)
	ldx	(s+)
	rsr

