;
;	Cast from char to int or uint
;
		.export __castc_l
		.export __castc_ul
		.setcpu 8085

		.code

__castc_l:
__castc_ul:
	mvi	h,0
	mov	a,l
	ora	a
	rp
	dcr	h
	mov	a,h
	sta	__hireg
	sta	__hreg+1
	ret
