;
;	32bit signed division and modulus. Basically a wrapper around
;	unsigned division that figures out the signs
;
	.export __divl
	.export __reml
	.export __diveql
	.export __remeql
;
;	At this point we have the entry frame as per div32x32
;
divs32x32:
	ld	a,=0
	st	a,:__tmp2		; sign track
	ld	ea,p1
	add	ea,=2
	jsr	negcheck
	ld	ea,p1
	add	ea,=6
	jsr	negcheck
	;	Values on stack are now positive and __tmp2 holds
	;	the sign info
	jmp	div32x32
mods32x32:
	ld	ea,p1
	add	ea,=2
	jsr	negcheck
	ld	a,=0
	st	a,:__tmp2		; sign track
	ld	ea,p1
	add	ea,=6
	jsr	negcheck
	;	Values on stack are now positive and __tmp2 holds
	;	the sign info
	jmp	div32x32


negcheck:
	ld	p3,ea
	ld	a,3,p3			; sign byte
	bp	pve			; no work
	xor	a,=0xFF
	st	a,3,p3
	ld	a,2,p3
	xor	a,=0xFF
	st	a,2,p3
	ld	ea,0,p3
	xor	a,=0xFF
	st	a,0,p3
	xch	a,e
	xor	a,=0xFF
	st	a,1,p3
	xch	a,e
	;	Complement done, now do the + 1
	add	ea,=1
	st	ea,0,p3
	ld	a,s
	bp	nocarry
	ld	ea,2,p3
	add	ea,=1
	st	ea,2,p3
nocarry:
	ild	a,:__tmp2
pve:
	ret

__divl:
	ld t,ea
	ld ea,:__hireg
	push ea
	ld ea,t
	push ea
	jsr div32x32
	pop p3
	pop p3
	; Stack now holds return and result above
	pop p3
	pop ea
	st ea,:__hireg

	ld a,:__tmp2
	and a,=1
	bz noneg
	pop ea
	push p3
	jsr __negatel
	ret
noneg:
	pop ea
	push p3
	ret

__reml:
	ld t,ea
	ld ea,:__hireg
	push ea
	ld ea,t
	push ea
	jsr div32x32
	ld t,ea
	ld a,:__tmp2
	bz nonegr
	ld ea,t
	jsr __negatel
	bra remout
nonegr:
	ld ea,t
remout:
	pop p3
	pop p3
	; Result is in hireg/ea
	pop p2	; return
	pop p3
	pop p3
	push p2
	ret

__diveql:
	;	2,p1 is the tos, build a working frame for the
	; 	divide call
	st ea,:__tmp
	ld ea,2,p1
	ld p3,ea
	ld ea,2,p3
	push ea
	ld ea,0,p3
	push ea
	push ea		; Dummy
	ld ea,:__hireg
	push ea
	ld ea,:__tmp
	push ea
	jsr div32x32

	ld a,:__tmp2
	and a,=1	; signs differed (1 or 3 negations)
	bp negdiv

	ld ea,12,p1	; Pointer to write back
	ld p3,ea
	
	pop p2
	pop p2		; discard divisor
	pop p2		; and dummy
	pop ea
	st ea,:__hireg
	st ea,2,p3
	pop ea		; actual result low
	st ea,0,p3
	pop p2		; return
	pop p3		; discard argument ptr
	push p2
	ret

negdiv:
	ld ea,12,p1
	ld p3,ea
	pop p2
	pop p2
	pop p2
	pop ea
	st ea,:__hireg
	pop ea
	jsr __negatel
	st ea,0,p3
	ld t,ea
	ld ea,:__hireg
	st ea,2,p3
	ld ea,t
	pop p2
	pop p3
	push p2
	ret

__remeql:
	;	2,p1 is the tos, build a working frame for the
	; 	divide call
	st ea,:__tmp
	ld ea,2,p1
	ld p3,ea
	ld ea,2,p3
	push ea
	ld ea,0,p3
	push ea
	push ea		; Dummy
	ld ea,:__hireg
	push ea
	ld ea,:__tmp
	push ea
	jsr div32x32

	; return is in hireg:ea
	ld t,ea
	ld ea,12,p1	; pointer to working var
	ld p3,ea
	ld a,:__tmp2
	bz nochange
	ld ea,t
	jsr __negatel
	ld t,ea
nochange:
	ld ea,t
out:
	st ea,0,p3
	ld ea,:__hireg
	st ea,2,p3
	ld ea,t
	pop p2
	pop p2		; discard divisor
	pop p2		; and dummy
	pop p2		; and dividend
	pop p2
	pop p2		; return
	pop p3		; discard argument ptr
	push p2
	ret

