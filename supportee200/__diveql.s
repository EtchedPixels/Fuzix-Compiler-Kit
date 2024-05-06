;
;	Do a 32bit divide unsigned between TOS ptr and hireg:B
;
	.setcpu	4
	.export __diveql
	.export __remeql
	.code

__diveql:
	stx	(-s)
	ldx	2(s)		; get the pointer
	; Push the data from the pointer
	lda	2(x)
	sta	(-s)
	lda	(x)
	sta	(-s)
	jsr	__divl
	; Result is in hireg:b
	lda	(__hireg)
	sta	(x)
	stb	2(x)
	; The call cleaned up the 4 bytes we pushed as an
	; argument. Just get X back
	ldx	(s+)
	rsr

__remeql:
	stx	(-s)
	ldx	2(s)		; get the pointer
	; Push the data from the pointer
	lda	2(x)
	sta	(-s)
	lda	(x)
	sta	(-s)
	jsr	__reml
	; Result is in hireg:b
	lda	(__hireg)
	sta	(x)
	stb	2(x)
	; The call cleaned up the 4 bytes we pushed as an
	; argument. Just get X back
	ldx	(s+)
	rsr
