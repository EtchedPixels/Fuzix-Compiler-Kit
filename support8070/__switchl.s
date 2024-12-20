
	.export __switchl
;
;	P3 points to our table
;
__switchl:
	ld t,ea			; save the value we seek
	ld ea,@2,p3
	st a,:__tmp		; count
	bz match
;
;	Walk the entries comparing with EA
;
loop:
	ld ea,t
	sub ea,@2,p3		; check if matches
	or a,e
	bnz nomatch
	ld ea,:__hireg
	sub ea,@2,p3
	or a,e
	bz match
	ld ea,@2,p3		; skip address
next:
	dld a,:__tmp
	bnz loop
match:
	ld ea,@2,p3		; get the function address into PC
	sub ea,=1
	ld p0,ea
nomatch:
	sub ea,@4,p3		; just inc p3 by 4
	bra next