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
	bp	carry1		; C set
	ld	ea,DIVID+2,p1	; high word of divider as we shift
	add	ea,DIVID+2,p1
word2:
	st	ea,DIVID+2,p1
	ld	a,s
	bp	carry2
	ld	ea,WORK+0,p1	; low word of working value
	add	ea,WORK+0,p1
word3:
	st	ea,WORK+0,p1
	ld	a,s
	bp	carry3
	ld	ea,WORK+2,p1	; High word of working value
	sl	ea
	st	ea,WORK+2,p1
subtest:
	;
	;	Now see if we need to adjust work as the division
	;	fits
	;
	sub	ea,DIVIS+2,p1	; Is high word >= divisor high
	ld	t,ea		; Save resulting high word
	bp	dobit_y
	bz	dobit
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
	bp	dobit_y1
	bz	dobit_y1
	bra	next
carry3:
	ld	ea,WORK+2,p1
	sl	ea		; Don't care about top bit
	add	a,=1
	bra	subtest
carry2:
	ld	ea,WORK+0,p1
	add	ea,=1		; order matters so C is right
	add	ea,WORK+0,p1
	bra	word3
carry1:
	ld	ea,DIVID+2,p1
	add	ea,=1
	add	ea,DIVID+2,p1
	bra	word2
