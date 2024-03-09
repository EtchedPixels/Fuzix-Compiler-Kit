; FIXME: we have call @irr so we should be ok

	.export __jmpr2
	.code

	; This way avoids messing with register indirection and working
	; our what our working register bank is
__jmpr2:
	push r3
	push r2
	ret
