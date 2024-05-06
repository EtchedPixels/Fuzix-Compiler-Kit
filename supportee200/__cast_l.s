;
;	cast int to long
;
	.setcpu	4
	.export __castc_l
	.export __castc_ul
	.export __cast_l
	.export __cast_ul

	.code
__castc_l:
__castc_ul:
	cla
	clrb bh
	orib bl,bl
	bp pve
	dcrb bh
	dca
	bra pve
__cast_l:
__cast_ul:
	cla
	ori b,b		; ensure m flag set right
	bp pve
	dca
pve:	sta (__hireg)
	rsr

