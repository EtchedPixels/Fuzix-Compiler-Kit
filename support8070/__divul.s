;	
;	Division 32 bit unsigned
;
	.export __divul
	.export __remul
	.export __divequl
	.export __remequl

__divul:
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
	pop ea
	push p3
	ret

__remul:
	ld t,ea
	ld ea,:__hireg
	push ea
	ld ea,t
	push ea
	jsr div32x32
	pop p3
	pop p3
	; Result is in hireg/ea
	pop p2	; return
	pop p3
	pop p3
	push p2
	ret

__divequl:
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
	ld ea,12,p1	; Pointer to write back
	
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

__remequl:
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
	ld ea,12,p1	; Pointer to write back
	ld p3,ea
	ld ea,:__hireg
	st ea,2,p3
	ld ea,t
	st ea,0,p3
	
	pop p2
	pop p2		; discard divisor
	pop p2		; and dummy
	pop p2		; and dividend
	pop p2
	pop p2		; return
	pop p3		; discard argument ptr
	push p2
	ret

