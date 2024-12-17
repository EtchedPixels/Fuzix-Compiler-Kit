	.export __shr_tw
	.export __shr_twu

	.code
;
;	Shift EA right by T
;

shr_via_u:
	xch a,e
__shr_twu:
	push ea
	ld ea,t
	and a,=15
	st a,:__tmp
	; TODO: smarts for 8 bit of shift etc
	bz shr_done
shr_nu:
	pop ea
	sr ea
	push ea
	dld a,:__tmp
	bz shr_done
	bra shr_nu
shr_done:
	pop ea
	ret

__shr_tw:
	xch a,e
	bp shr_via_u	; use the unsigned shift for positives
	push ea
	ld ea,t
	and a,=15
	st a,:__tmp
	bz shr_done
shr_n:
	pop ea
	; No 16bit sra so this is messy
	sr ea
	xch a,e
	or a,=0x80
	xch a,e
	push ea
	dld a,:__tmp
	bz shr_done
	bra shr_n

	