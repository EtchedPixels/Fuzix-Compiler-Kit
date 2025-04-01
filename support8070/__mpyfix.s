;
;	Not clear if we need this but the manual states (2.2.5.6) that
;	T must be +ve range (so top bit clear)
;

	.export __mpyfix

__mpyfix:
	st	ea,:__tmp
	ld	ea,t
	xch	a,e
	; Right side is clear on top bit - can just use MPY
	bp	allgood
	xch	a,e
	and	a,=1
	bnz	shiftmula
	; Low bit is 0, shift it right, do the mul, shift it left
	ld	ea,t
	sr	ea
	ld	t,ea
	ld	ea,:__tmp
	mpy	ea,t
	ld	ea,t
	sl	ea
	ret
shiftmula:
	; Low bit is 1,  shift it right, do the mul. shift it left
	; add the low bit of the multiply in
	ld	ea,t
	sr	ea
	ld	t,ea
	ld	ea,:__tmp
	mpy	ea,t
	ld	ea,t
	sl	ea
	add	ea,:__tmp
	ret
allgood:
	ld	ea,:__tmp
	mpy	ea,t
	ld	ea,t
	ret
