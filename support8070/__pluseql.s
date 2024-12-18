;
;	(TOS.L) += hireg:EA
;
	.export __pluseql

__pluseql:
	pop p2
	pop p3
	push p2
	; Extract pointer from stack
	add ea,0,p3
	st ea,0,p3
	ld t,ea
	rrl a		; carry is a hand crank job
	bp noinc
	ld ea,2,p3
	add ea,=1	; carry the word
	bra next
noinc:
	ld ea,2,p3
next:
	add ea,:__hireg	; add the upper words
	st ea,:__hireg
	st ea,2,p3	; write the upper word back
	ld ea,t		; get the low word back
	ret
