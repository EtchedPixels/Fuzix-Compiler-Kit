	.export __sub_tw
	.export __sub_twu

	.code

__sub_tw:
__sub_twu:
	; EA - T
	xch ea,t
	st ea,@__tmp
	xch ea,t
	sub ea,@__tmp
	ret

