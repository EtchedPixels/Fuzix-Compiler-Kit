;
;	Helpers for stuff using with -Os
;
	.export __signedcomp
	.export __signed8comp
	.export __unsignedcomp

	.code

__signed8comp:
	xch	a,e		; get the sign into E for 8bits
__signedcomp:
	ld	a,s		; Get flags into A
	sl	a		; Align overflow with top bit
	xor	a,e		; Top bit is now sign xor overflow *.
	sl	ea		; Move it into low bit of E
	ld	a,=0		; Clear A
	xch	a,e		; Swap 0 into E and result a low bit of A
	and	a,=1		; Clear rest of A
	ret

__unsignedcomp:
	ld	ea,=0		; Clear EA
	rrl	a		; Rotate carry into top of A (no rll a alas)
	sl	ea		; Shift it into low bit of E
	xch	a,e		; Swap A and E so its now low bit of A
	ret
