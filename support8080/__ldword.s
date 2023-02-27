;
;	Load word from further off stack
;
		.export __ldword

		.setcpu 8080
		.code

__ldword:
	pop	h
	mov	e,m
	inx	h
	mvi	d,0
	push	h
	dad	sp
	mov	a,m
	inx	h
	mov	h,m
	mov	l,a
	ret

