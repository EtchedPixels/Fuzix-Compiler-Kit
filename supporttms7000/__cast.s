	.export __castc_
	.export __castc_u
	.export __castc_l
	.export __castc_ul
	.export __cast_l
	.export __cast_ul

	.code

; We have no sex instructions so we have to do this the hard way

__castc_:
__castc_u:
__castc_l:
__castc_ul:
	or r5,r5
	jn negc
	clr r4
	clr r3
	clr r2
	rets
negc:
	movd %0xFFFF,r3
	mov r2,r4
	rets

__cast_l:
__cast_ul:
	or r4,r4
	jnc negw
	clr r2
	clr r3
	rets
negw:
	movd %0xFFFF,r3
	rets
