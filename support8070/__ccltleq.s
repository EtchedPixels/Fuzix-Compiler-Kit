;
;	TOS.L <= __hireg:EA
;
	.export __ccltleq

__ccltleq:
	st ea,:__tmp
	ld ea,4,p1	; high word
	sub ea,:__hireg
	ld t,ea
	bp borrow_hi
	; Result cannot be 0 on this path
	ld a,s
	and a,=0x40
	bz false
	bra true
borrow_hi:
	; Firstly check if our result was 0 ?
	or a,e
	bz check_low
	; Now we need to check OV and borrow
	ld a,s
	and a,=0x40	; Get the OV flag
	bz true
false:
	ld ea,=0
out:
	pop p3
	pop p2
	pop p2
	push p3
	ret
true:
	ld ea,=1
	bra out

check_low:
	ld ea,2,p1
	sub ea,:__tmp
	or a,e
	bz true
	rrl a
	bp true
	bra false