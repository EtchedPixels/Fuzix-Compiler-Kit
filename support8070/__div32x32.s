;
;	Divide two values on the top of the stack
;

WORK	.equ	0		; 0-3, 4-5 is return
DIVIS	.equ	6		; 6-9, 10-11 is return
DIVID	.equ	12		; 12-15

	.export div32x32
	.code

cleared_bit:
	ld	ea,DIVID+0,p1
	sl	ea
	st	ea,DIVID+0,p1
	xch	a,e
	bp	nocarry4
	ld	ea,DIVID+2,p1
	sl	ea
	add	a,=1
	st	ea,DIVID+2,p1
	bra	no_subtract
nocarry4:
	ld	ea,DIVID+2,p1
	sl	ea
	st	ea,DIVID+2,p1
	bra	no_subtract
div32x32:
	ld	a,=32
	st	a,:__tmp

	ld	ea,=0
	push	ea
	push	ea		; set up working value

loop:
	ld	a,DIVID+3,p1
	bp	cleared_bit

	ld	ea,DIVID+0,p1	; do the low word
	sl	ea
	st	ea,DIVID+0,p1
	xch	a,e
	bp	nocarry

	ld	ea,DIVID+2,p1
	sl	ea
	add	a,=1		; carry
ncr:
	st	ea,DIVID+2,p1

	ld	ea,WORK+0,p1
	sl	ea
	add	a,=1
	st	ea,WORK+0,p1
	xch	a,e
	bp	nocarry2
	ld	ea,WORK+2,p1
	sl	ea
	add	a,=1
ncr2:
	st	ea,WORK+2,p1

	; Shifts done, now see if the value is now bigger, if not clear the
	; low bit of work

	ld	ea,WORK+2,p1
	sub	ea,DIVIS+2,p1
	bp	no_subtract
	bnz	do_subtract
	ld	ea,WORK,p1
	sub	ea,DIVIS,p1
	bp	no_subtract

sbr1:
	; Do the subtraction
	; We just did the low half
	st	ea,WORK,p1
	ld	a,s
	bp	nocarry3
	ld	ea,WORK+2,p1
	sub	ea,=1
ncr3:
	sub	ea,DIVIS+2,p1
	st	ea,WORK+2,p1

	ild	a,WORK+0,p1

no_subtract:
	; And loop
	dld 	a,:__tmp
	bnz	loop

	; Result is now in 
	; Remainder is is in work so extract it
	pop	p3	;	low
	pop	ea
	st	ea,:__hireg
	xch	ea,p3	;	low into EA
	ret

nocarry:
	ld	ea,DIVID+2,p1
	sl	ea	
	bra	ncr
nocarry2:
	ld	ea,WORK+2,p1
	sl	ea	
	bra	ncr2
nocarry3:
	ld	ea,WORK+2,p1
	bra	ncr3
do_subtract:
	ld	ea,WORK,p1
	sub	ea,DIVIS,p1
	bra	sbr1
