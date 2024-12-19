;
;	TOS.L << EA
;
	.export __shll
	.export __shleql

__shll:
	and a,=31
	bz nowork
	ld t,ea
	and a,=16
	bnz slide16
test8:
	ld ea,t
	and a,=8
	bnz bytes
final:
	ld ea,t
	and a,=7
	bz nowork
	st a,:__tmp
; Left shifting 32bit - icky

loop:
	ld ea,2,p1
	add ea,2,p1
	st ea,2,p1
	rrl a
	bp noadd
	ld ea,4,p1
	add ea,4,p1
	add ea,=1
	st ea,4,p1
	bra next
noadd:
	ld ea,4,p1
	add ea,4,p1
	st ea,4,p1
next:
	dld a,:__tmp
	bnz loop
nowork:
	pop p2	; return
	pop ea	; low
	ld t,ea
	pop ea	; high
	st ea,:__hireg
	ld ea,t
	push p2
	ret
slide16:
	ld ea,2,p1
	st ea,4,p1
	ld ea,=0
	st ea,2,p1
	ld ea,t
	and a,=15
	ld t,ea
	bra test8
bytes:
	ld ea,3,p1
	st ea,4,p1
	ld a,2,p1
	st a,3,p1
	ld a,=0
	st a,2,p1
	bra final

__shleql:
	pop p3
	pop p2
	push p3
	push p2
	ld t,ea
	ld ea,2,p2
	push ea
	ld ea,0,p2
	push ea
	ld ea,t
	jsr __shll
	; Will have popped the arg
	pop p2
	st ea,0,p2
	ld t,ea
	ld ea,:__hireg
	st ea,2,p2
	ld ea,t
	ret
