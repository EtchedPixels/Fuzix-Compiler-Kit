;
;	(TOS) >> EA
;
	.export __shreqc
	.export __shrequc

__shrequc:
	and a,=7	; wrap by bit count
	ld e,a
	pop p2		; return
	pop p3		; ptr
	bz noshift
	ld a,0,p3
pve:
	xch a,e
loop:
	xch a,e
	sr a
	xch a,e
	sub a,=1
	bnz loop
out:
	xch a,e
	st a,0,p3
	push p2
	ret

noshift:
	ld a,0,p3
	push p2
	ret

__shreqc:
	and a,=7
	ld e,a
	pop p2
	pop p3
	bz noshift
	ld a,0,p3
	bp pve
;
;	Need to keep setting top bit
;
	xch a,e
loop1:
	xch a,e
	sr a
	or a,=0x80
	xch a,e
	dld a,:__tmp
	bnz loop1
	bra out
