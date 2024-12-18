;
;	TOS >= hireg:EA unsigned
;
	.export __ccgtequl

__ccgtequl:
	st ea,:__tmp
	ld ea,4,p1		; high word
	sub ea,:__hireg
	or a,e
	bz test_low
outc:
	rrl a			; C into top bit of A
	bp false		; Borrow bit so clear means false
true:
	ld ea,=1
out:
	pop p2
	pop p3
	pop p3
	push p2
	ret
false:
	ld ea,=0
	bra out

test_low:
	ld ea,2,p1
	sub ea,:__tmp
	or a,e
	bz true
	bra outc
