;
;	(TOS.L) -- by hireg:EA
;
	.export __postdecl

__postdecl:
	pop p2
	pop p3
	push p2
	; Extract pointer from stack
	st ea,:__tmp	; save adjustment
	ld ea,0,p3
	push ea		; save original value
	sub ea,:__tmp
	st ea,0,p3
	rrl a		; carry is a hand crank job
	bp dodec
	ld ea,2,p3
	push ea
	bra next
dodec:
	ld ea,2,p3
	push ea
	sub ea,=1
next:
	sub ea,:__hireg	; add the upper words
	st ea,2,p3	; write the upper word back
	pop ea		; high half
	st  ea,:__hireg
	pop ea		; low half
	ret
