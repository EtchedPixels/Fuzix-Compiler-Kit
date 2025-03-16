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
	push ea		; save divisor
	ld ea,=0
	push ea		; push a working 0
	ld a,=16
	push a		; push a 16 count

	; At this point
	;
	; 0,p1		count
	; 1,p1		working
	; 3,p1		divisor
	; 5,p1		return address
	; 7,p1		dividend
loop:
	; The 8070 sl ea is particuarly annoying as it throws the old
	; top bit away so we have to check it before shifting
	ld ea,7,p1	; working <<= 1
	xch a,e		; so we can test it
	bp shiftin0	; was set so shift into 1,p1
	xch a,e		; back in right order
	sl ea
	st ea,7,p1
	ld ea,1,p1	; now shift working with low bit 0
	sl ea
	add a,=1	; set the low bit
	bra check
shiftin0:
	xch a,e		; put back th right way around
	sl ea
	st ea,7,p1	; shift 0 into the other word
	ld ea,1,p1
	sl ea
check:
	st ea,1,p1	; save the working value back
	sub ea,3,p1	; is it bigger than the divisor ?
	rrl a		; get carry bit into bit 7 of A
	bp skip		; skip if clear
	ld ea,1,p1	; can't test carry without eating ea easily
	sub ea,3,p1	; so do it again
	st ea,1,p1	; adjusted working value back
	ild a,7,p1	; set low bit of dividend (will be 0 before ild)
skip:
	dld a,0,p1
	bnz loop

	; EA is remainder
	; Division result is in 7,p1 (caller arg), save in T

	ld t,7,p1

	pop a		; count
	pop ea		; working
	pop p2		; divisor
	pop p3		; return
	pop p2		; argument
	push p3
	ret
;
;	Some division we can use the div op for but only some 8(
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
