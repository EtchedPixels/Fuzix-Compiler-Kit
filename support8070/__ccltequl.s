;
;	TOS <= hireg:EA unsigned
;
	.export __ccltequl

__ccltequl:
	st ea,:__tmp
	ld ea,4,p1		; high word
	sub ea,:__hireg
	or a,e
	bz test_low
outc:
	rrl a			; C into top bit of A
	bp true			; Borrow bit so clear means true
	ld ea,=0
out:
	pop p2
	pop p3
	pop p3
	push p2
	ret
true:
	ld ea,=1
	bra out

test_low:
	ld ea,2,p1
	sub ea,:__tmp
	or a,e
	bz true
	bra outc
