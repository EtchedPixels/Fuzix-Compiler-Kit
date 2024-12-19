;
;	TOS.L <= __hireg:EA
;
	.export __cclteql

__cclteql:
	st ea,:__tmp
	ld ea,4,p1	; high word
	sub ea,:__hireg
	ld t,ea
	xch a,e		; so we can check sign
	bp positive
	; Result cannot be 0 on this path
	ld a,s
	and a,=0x40
	bz true		; S = 1 O = 0
	bra false
positive:
	; Firstly check if our result was 0 ?
	or a,e
	bz check_low
	; Now we need to check OV and borrow
	ld a,s
	and a,=0x40	; Get the OV flag
	bnz true	; S = 0 O = 1
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
