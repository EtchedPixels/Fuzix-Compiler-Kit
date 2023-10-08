	.65c816
	.a16
	.i16

	.export div32x32

;
;	32bit unsigned divide. Used as the core for the actual C library
;	division routines. It expects to be called with the parameters
;	offsets from X, and uses tmp/tmp2/tmp3.
;
;	tmp2/tmp3 end up holding the remainder
;
;	On entry the stack frame referenced by X looks like this
;
;	4-7	32bit dividend (C compiler TOS)
;	0-3	32bit divisor
;
;	The one trick here is that to save space and time we start
;	with DIVID,x hoilding the 32bit input value (N in the usual
;	algorithm description). Each cycle we take the top bit of N,
;	we shift it left discarding this bit from DIVID,x and we shift the
;	resulting Q(n) bit into the bottom. After 32 cycles we throw N(0)
;	out and have shifted all of Q into the result.
;

DIVIS	.equ	0
DIVID	.equ	4

	.export div32x32
	.code

div32x32:
	ldx #32			; 32 iterations for 32 bits
	stx @tmp
	; Clear the working register (tmp2/tmp3)
	; R = 0;
	stz @tmp2
	stz @tmp3
	lda @tmp2
	tyx			; We can only use X as the index for rol
loop:	; Shift the dividend left and set bit 0 assuming that
	; R >= A
	sec
	rol DIVID,x
	rol DIVID+1,x
	rol DIVID+2,x
	rol DIVID+3,x
	; N(i) is now in carry
	; R <<= 1; R(0) = N(i)
	; Capture into the working register
	rol @tmp2		; capturing high bit into the
	rol @tmp2+1		; working register bottom
	rol @tmp3
	rol @tmp3+1
	; Do a 32bit subtract but skip writing the high 16bits
	; back until we know the comparison
	;
	; R - D
	;
	lda @tmp2
	sec
	sbc DIVIS,y
	sta @tmp2
	lda @tmp3
	sbc DIVIS+2,y
	; Want to subtract (R - D >= 0)
	bcc skip
	; No subtract, so put back the low 16bits we mushed
	lda @tmp2
	clc
	adc DIVIS,y
	sta @tmp2
	; We guessed the wrong way for Q(i). Clear Q(i) which is
	; in the lowest bit and we know is set so using dec is safe
	dec DIVID,x
done:
	dec @tmp
	bne loop
	rts
		; We do want to subtract - write back the other bits
skip:
	; R -= D
	sta @tmp3
	dec @tmp
	bne loop
	rts
