;
;	Cast from char to int or uint
;
		.export __castc_
		.export __castc_u
		.setcpu 8085

		.code

__castc_:
__castc_u:
	mvi	h,0
	mov	a,l
	ora	a
	rp
	dcr	h
	ret
