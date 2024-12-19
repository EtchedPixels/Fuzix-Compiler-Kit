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

	ld ea,0
	push ea

	ld ea,4,p1
	jsr negate
	ld t,ea

	ld ea,:__tmp
	jsr negate

	div ea,t
	add ea,0,p1

	pop p3		; sign track
	pop p3		; return
	pop p2		; argument
	push p3
	ret

negate:
	xch a,e
	bp noneg
	xor a,=0xFF
	xch a,e
	xor a,=0xFF
	add ea,=1
	push ea
	ld a,=0x80
	or a,3,p1	; The upper byte of the sign track
	st a,3,p1
	pop ea
noneg:
	xch a,e
	ret

__rem:
	push p3		; Dummy word for negate to use

	jsr negate
	st ea,:__tmp

	ld ea,=0
	st ea,0,p1	; Ovewrwrite dummy word

	ld ea,4,p1
	jsr negate	; track sign

	ld t,ea
	ld ea,:__tmp
	push ea
	ld ea,t

	jsr __remu	; Do unsigned remainder

	ld t,ea
	pop ea
	bz pve
	ld ea,t
	jsr negate
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
