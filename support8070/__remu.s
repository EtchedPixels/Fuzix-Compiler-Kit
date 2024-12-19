;
;	No hardware remainder so do this the
;	classic way
;
;	TOS / EA
;
	.export __divu
	.export __remu

	.export __divequ
	.export __remequ

	.export __divequc
	.export __remequc

__remu:
	push ea
	ld ea,=0
	push ea
	ld a,=16
	push a
	; At this point
	;
	; 0,p1		count
	; 1,p1		working
	; 3,p1		divisor
	; 5,p1		return address
	; 7,p1		dividend
loop:
	ld ea,7,p1
	sl ea
	st ea,7,p1
	xch a,e
	bp notset
	ld ea,1,p1
	sl ea
	add a,=1
check:
	st ea,1,p1
	sub ea,3,p1
	rrl a
	bp skip
	ld ea,1,p1
	add a,=1
	st ea,1,p1
skip:
	dld a,0,p1
	bnz loop

	; EA is remainder
	; Division result is in 7,p1 (caller arg), save in T

	ld t,7,p1

	pop a		; count
	pop p2		; working
	pop p2		; divisor
	pop p3		; return
	pop p2		; argument
	push p3
	ret
notset:
	ld ea,1,p1
	sl ea	
	bra check

;
;	Some divison we can use the div op for but only some 8(
;
__divu:
	xch a,e
	bp hw_ok
	xch a,e
	ld t,ea
	ld ea,2,p1	; get TOS argument
	push ea		; build a new frame
	ld ea,t
	jsr __remu
	ld ea,t		; division result
	pop p3		; frame we built
	pop p2		; return
	pop p3		; argument
	push p2
	ret
hw_ok:
	xch a,e
	ld t,ea
	ld ea,2,p1
	div ea,t
	pop p2
	pop p3
	push p2
	ret

__remequ:
	pop p3
	pop p2
	push p3
	ld t,ea
	ld ea,0,p2
	push p2
	push ea
	ld ea,t
	jsr __remu
	pop p2
	st ea,0,p2
	ret

__remequc:
	pop p3
	pop p2
	push p3
	ld t,ea
	ld ea,=0
	ld a,0,p2
	push p2
	push ea
	ld ea,t
	jsr __remu
	pop p2
	st a,0,p2
	ret

__divequ:
	pop p3
	pop p2
	push p3
	ld t,ea
	ld ea,0,p2
	push p2
	push ea
	ld ea,t
	jsr __divu
	pop p2
	st ea,0,p2
	ret

__divequc:
	pop p3
	pop p2
	push p3
	ld t,ea
	ld ea,=0
	ld a,0,p2
	push p2
	push ea
	ld ea,t
	jsr __divu
	pop p2
	st a,0,p2
	ret
