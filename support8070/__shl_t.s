	.export __shr_tw
	.export __shr_twu

	.code
;
;	Shift EA right by T
;

shr_via_u:
	xch a,e
__shr_twu:
	; TODO: smarts for 8 bit of shift etc
	xch ea,t
	bz shr_done
shr_nu:
	xch ea,t
	sr ea
	xch ea,t
	sub a,=1
	bz shr_done
	bra shr_nu
shr_done:
	xch ea,t
	ret

__shr_tw:
	xch a,e
	bp shr_via_u	; use the unsigned shift for positives
shr_n:
	; No 16bit srl so this is messy
	xch ea,t
	srl ea
	xch a,e
	or a,=0x80
	xch a,e
	xch ea,t
	sub a,=1
	bz shr_done
	bra shr_n
	