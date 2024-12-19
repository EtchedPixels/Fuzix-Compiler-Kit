;
;	(TOS) << EA
;
	.export __shreq
	.export __shrequ

__shrequ:
	pop p3
	pop p2
	push p3

	and a,=15
	bz nowork

	st a,:__tmp

	ld t,0,p2
next:
	ld ea,t
	sr ea
	ld t,ea
	dld a,:__tmp
	bnz next

	ld ea,t
	st ea,0,p2
	ret
nowork:
	ld ea,0,p2
	ret

__shreq:
	pop p3
	pop p2
	push p3

	and a,=15
	bz nowork

	st a,:__tmp

	ld t,0,p2
	ld a,1,p2
	bp next		; via the unsigned

	; Keep setting high bit

next1:
	ld ea,t
	sr ea
	add ea,=0x8000
	ld t,ea
	dld a,:__tmp
	bnz next1
	ld ea,t
	st ea,0,p2
	ret

	