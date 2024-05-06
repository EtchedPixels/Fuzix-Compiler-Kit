;
;	Do a 32bit divide unsigned
;
	.setcpu	4
	.export __divul
	.export __remul
	.code

__divul:
	stb	(-s)
	ldb	(__hireg)
	stb	(-s)
	jsr	div32x32
	; Result is in the original stacked value
	lda	6(s)
	ldb	8(s)
	sta	(__hireg)
	lda	8	; clean up frame
	add	a,s
	rsr

__remul:
	stb	(-s)
	ldb	(__hireg)
	stb	(-s)
	jsr	div32x32
	sta	(__hireg)	
	lda	8
	add	a,s
	rsr
