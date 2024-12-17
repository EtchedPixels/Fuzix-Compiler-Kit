	.export __shl_tw
	.export __shl_twu

	.code
;
;	Shift EA left by T
;

__shl_twu:
__shl_tw:
	push ea
	ld ea,t
	and a,=15
	st a,:__tmp
	; TODO: smarts for 8 bit of shift etc
	bz shl_done
shl_nu:
	pop ea
	sl ea
	push ea
	dld a,:__tmp
	bz shl_done
	bra shl_nu
shl_done:
	pop ea
	ret
