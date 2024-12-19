;
;	(TOS) << EA
;
	.export __shleqc

__shleqc:
	pop p3
	pop p2
	push p3

	and a,=7
	bz nowork

	ld e,a

	ld a,0,p2
	xch a,e
next:
	xch a,e
	sl a
	xch a,e
	sub a,=1
	bnz next

	xch a,e
	st a,0,p2
	ret
nowork:
	ld a,0,p2
	ret
