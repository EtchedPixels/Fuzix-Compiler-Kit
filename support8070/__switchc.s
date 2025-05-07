	.export __switchc
;
;	P3 points to our table
;
__switchc:
	ld t,ea			; save the value we seek
	ld ea,@2,p2
	st a,:__tmp		; count
	bz match
;
;	Walk the entries comparing with EA
;
loop:
	ld ea,t
	sub a,@1,p2		; check if matches
	bz match
	ld ea,@2,p2		; skip address
	dld a,:__tmp
	bnz loop
match:
	ld ea,@2,p2		; get the function address into PC
	sub ea,=1
	ld p0,ea
