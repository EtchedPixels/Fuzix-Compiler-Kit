;
;	Divide two values on the top of the stack
;

WORK	.equ	0		; 0-3, 4-5 is return
DIVIS	.equ	6		; 6-9, 10-11 is return
DIVID	.equ	12		; 12-15

	.export div32x32
	.code

div32x32:
	ld	a,=32
	st	a,:__tmp	; counter

	;	Clear working register (will 0-3,p1)
	ld	ea,=0
	push	ea
	push	ea

	; We are now ready to begin

loop:
	; We need to do a 32bit rotate of DIVID into 32bit DIVID
	; On the 807x it's a bit of a mess as sl ea throws away the
	; top bit and you can't add ea,ea
	ld	ea,DIVID+0,p1	; low word
	add	ea,DIVID+0,p1	; low word shifted, C is valid
	st	ea,DIVID+0,p1
	ld	a,s		; get the C flag
	bp	nocarry1		; C set
	ld	ea,DIVID+2,p1	; high word of divider as we shift
	add	ea,=1
word2:
	add	ea,DIVID+2,p1
	st	ea,DIVID+2,p1
	ld	a,s
	bp	nocarry2
	ld	ea,WORK+0,p1	; low word of working value
	add	ea,=1
word3:
	add	ea,WORK+0,p1
	st	ea,WORK+0,p1
	ld	a,s
	bp	nocarry3
	ld	ea,WORK+2,p1	; High word of working value
	add	ea,=1
subtest:
	add	ea,WORK+2,p1
	st	ea,WORK+2,p1
	;
	;	Now see if we need to adjust work as the division
	;	fits
	;
	sub	ea,DIVIS+2,p1	; Is high word >= divisor high
	ld	t,ea		; Save resulting high word
	bz	dobit
	bp	dobit_y
next:
	dld	a,:__tmp	; count down
	bnz	loop
	;
	; Job is done. Result is in work which is not safe on return
	; Remainder is on stack so is safe
	;
	pop	ea		; low word
	ld	t,ea
	pop	ea		; high word
	st	ea,:__hireg	; into expected place
	ld	ea,t
	;
	; Stack is now tidied so we can just
	;
	ret

	;
	;	Do the 32bit subtract and save it
	;	Set low bit in work (currently 0)
	;
dobit_y:
	ld	ea,WORK+0,p1
	sub	ea,DIVIS+0,p1
dobit_y1:
	st	ea,WORK+0,p1
	ld	a,s
	bp	carry4
	ld	ea,t
dobitr:
	st	ea,WORK+2,p1
	ild	a,DIVID+0,p1	; set low bit of DIVID
	bra	next
carry4:
	ld	ea,t
	sub	ea,=1		; carry the subtraction
	bra	dobitr
	;
	;	High bits matched
	;
dobit:
	ld	ea,WORK+0,p1
	sub	ea,DIVIS+0,p1
	xch	ea,p2
	ld	a,s
	bp	next	; not yet big enough
dobit_yp2:
	xch	ea,p2
	bra	dobit_y1
nocarry3:
	ld	ea,WORK+2,p1
	bra	subtest
nocarry2:
	ld	ea,WORK+0,p1
	bra	word3
nocarry1:
	ld	ea,DIVID+2,p1
	bra	word2
