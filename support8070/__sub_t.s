	.export __sub_tw
	.export __sub_twu

	.code

__sub_tw:
__sub_twu:
	push ea
	ld ea,t			; no useful way to store T
	st ea,:__tmp
	pop ea
	sub ea,:__tmp
	ret

