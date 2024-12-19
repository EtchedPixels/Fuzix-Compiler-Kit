	.export __shr
	.export __shru

;	TOS << EA

__shru:
	and a,=15
	bz nowork

	st a,:__tmp
	pop p2
	pop ea
	push p2

	ld t,ea
loop:
	ld ea,t
	sr ea
	ld t,ea
	dld a,:__tmp
	bnz loop

	ld ea,t
	ret

nowork:
	pop p2
	pop ea
	push p2
	ret

__shr:
	and a,=15
	bz nowork

	st a,:__tmp
	pop p2
	pop ea
	push p2

	ld t,ea
	xch a,e
	bp loop

loop1:
	ld ea,t
	sr ea
	add ea,=0x8000
	ld t,ea
	dld a,:__tmp
	bnz loop1

	ld ea,t
	ret
