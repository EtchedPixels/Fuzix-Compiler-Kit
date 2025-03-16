;
;	Division. The hardware can do the +ve/+ve 15 bit quadrant
;	but the rest is our problem
;
	.export __div
	.export __diveq
	.export __diveqc

	.export __rem
	.export __remeq
	.export __remeqc

__div:
	st ea,:__tmp

	ld ea,=0
	push ea

	ld ea,:__tmp
	jsr negate
	ld t,ea

	ld ea,4,p1
	jsr negate

	div ea,t

	ld t,ea
	ld a,0,p1
	and a,=1	; check if odd number
	bz nonegb

	ld ea,t
	xch a,e		; donegate needs it reversed
	jsr donegate
	bra out
nonegb:
	ld ea,t
out:
	pop p3		; sign track
	pop p3		; return
	pop p2		; argument
	push p3
	ret

negate:
	xch a,e
	bp noneg
donegate:
	xor a,=0xFF
	xch a,e
	xor a,=0xFF
	add ea,=1
	push a
	ild a,3,p1	; count sign changes
	pop a
	ret
noneg:
	xch a,e
	ret

__rem:
	push p3		; Dummy word for negate to use

	jsr negate
	st ea,:__tmp
	ld t,ea		; Put int T for the div operation

	pop p3
	ld p3,=0
	push p3		; sign count we actually care about

	ld ea,4,p1
	jsr negate	; negate dividend

	st ea,4,p1	; save +ve version as we will need it in a moment

	; EA / T

	div ea,t	; get the divide for the +ve quadrant
	; EA now holds the dividend
	ld t,:__tmp	; get the divisor back
	mpy ea,t	; and multiply to get the integer part
	ld ea,t		; into EA

	st ea,:__tmp	; save it
	ld ea,4,p1	; get the positive of the original value
	sub ea,:__tmp	; get the remainder part into EA

	ld t,ea		; save value

	pop ea		; sign info

	bz pve		; nope
	ld ea,t
	xch a,e		; expects it swapped
	push p3		; dummy
	jsr donegate
	pop p3
	pop p3		; ret
	pop p2		; value
	push p3
	ret
pve:
	ld ea,t
	pop p3
	pop p2
	push p3
	ret

__diveq:
	; Same idea but with (TOS)
	pop p3
	pop p2
	push p3
	ld t,ea
	ld ea,0,p2
	push p2
	push ea
	ld ea,t
	jsr __div
	pop p2
	st ea,0,p2
	ret

__diveqc:
	pop p3
	pop p2
	push p3
	ld t,ea
	ld ea,=0
	ld a,0,p2
	push p2
	push ea
	ld ea,t
	jsr __div
	pop p2
	st a,0,p2
	ret

__remeq:
	; Same idea but with (TOS)
	pop p3
	pop p2
	push p3
	ld t,ea
	ld ea,0,p2
	push p2
	push ea
	ld ea,t
	jsr __rem
	pop p2
	st ea,0,p2
	ret

__remeqc:
	pop p3
	pop p2
	push p3
	ld t,ea
	ld ea,=0
	ld a,0,p2
	push p2
	push ea
	ld ea,t
	jsr __rem
	pop p2
	st a,0,p2
	ret
