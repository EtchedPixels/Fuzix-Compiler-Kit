;
;	(TOS) >> EA
;
	.export __shreq
	.export __shrequ

__shrequ:
	and a,=15	; wrap by bit count
	st a,:__tmp
	pop p2		; return
	pop p3		; ptr
	bz noshift
	ld t,0,p3
loop:
	ld ea,t
	sr ea
	ld t,ea
	dld a,:__tmp
	bnz loop
	ld ea,t
	st ea,0,p3
	push p2
	ret

noshift:
	ld ea,0,p3
	push p2
	ret

via_u:
	xch a,e
	ld t,ea
	bra loop

__shreq:
	and a,=15	; wrap by bit count
	st a,:__tmp
	pop p2		; return
	pop p3		; ptr
	bz noshift
	ld ea,0,p3
	xch a,e
	bp via_u
	xch a,e
	ld t,ea
loop1:
	ld ea,t
	sr ea
	xch a,e
	or a,=0x80
	xch a,e
	ld t,ea
	dld a,:__tmp
	bnz loop1
	ld ea,t
	st ea,0,p3
	push p2
	ret

	
	
