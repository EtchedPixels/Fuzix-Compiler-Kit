;
;	(TOS.L) ++ by hireg:EA
;
	.export __postincl

__postincl:
	pop p2
	pop p3
	push p2
	; Extract pointer from stack
	st ea,:__tmp	; save adjustment
	ld ea,0,p3
	push ea		; save original value
	add ea,:__tmp
	st ea,0,p3
	rrl a		; carry is a hand crank job
	bp noinc
	ld ea,2,p3
	push ea
	add ea,=1	; carry the word
	bra next
noinc:
	ld ea,2,p3
	push ea
next:
	add ea,:__hireg	; add the upper words
	st ea,2,p3	; write the upper word back
	pop ea		; high half
	st  ea,:__hireg
	pop ea		; low half
	ret
