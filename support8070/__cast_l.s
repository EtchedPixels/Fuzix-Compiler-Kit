;
;	Integer to long cast
;
	.export __castc_l
	.export __cast_l
	.export __cast_ul
	.export __castu_l
	.export __castu_ul
	.export __castc_u
	.export __castuc_u
	.export __castuc_l
	.export __castuc_ul

__castc_l:
	bp __castuc_l
	xch a,e
	ld a,=0xFF
	xch a,e
__cast_l:
__cast_ul:
	ld t,ea
	xch a,e
	bp clhigh
	ld ea,=0xFFFF
out:
	st ea,:__hireg
	ld ea,t
	ret
__castuc_l:
__castuc_ul:
	xch a,e
	ld a,=0
	xch a,e
__castu_l:
__castu_ul:
	ld t,ea
clhigh:
	ld ea,=0
	bra out

__castc_u:
	bp zeroit
	xch a,e
	ld a,=0xFF
	xch a,e
	ret

__castuc_u:
zeroit:
	xch a,e
	ld a,=0x00
	xch a,e
	ret
	
