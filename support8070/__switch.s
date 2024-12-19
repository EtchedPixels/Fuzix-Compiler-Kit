
	.export __switch
;
;	P3 points to our table
;
__switch:
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
	bz match
	ld ea,@2,p3		; skip address
	dld a,:__tmp
	bnz loop
match:
	ld ea,@2,p3		; get the function address into PC
	sub ea,=1
	ld p0,ea

