;
;	32bit unsigned divide. Used as the core for the actual C library
;	division routines. It expects to be called with the parameters
;	offsets from X, and uses tmp/tmp2/tmp3.
;
;	tmp2/tmp3 end up holding the remainder
;
;	On entry the stack frame referenced by X looks like this
;
;	6-9	32bit dividend (C compiler TOS)
;	4,5	A return address
;	0-3	32bit divisor
;
;	The one trick here is that to save space and time we start
;	with DIVID,x hoilding the 32bit input value (N in the usual
;	algorithm description). Each cycle we take the top bit of N,
;	we shift it left discarding this bit from DIVID,x and we shift the
;	resulting Q(n) bit into the bottom. After 32 cycles we throw N(0)
;	out and have shifted all of Q into the result.
;
;	TODO: convert to be re-entrant by putting tmp2/tmp3 on stack
;


DIVIS		.equ	0
;		4/5,x are a return address and it's easier to just leave
;		the gap
DIVID		.equ	6

		.export div32x32
		.code

div32x32:
		ldab #32		; 32 iterations for 32 bits
		stab @tmp
		clra
		clrb
		; Clear the working register (tmp2/tmp3)
		; R = 0;
		clr @tmp2
		clr @tmp2+1
		clr @tmp3
		clr @tmp3+1
loop:		; Shift the dividend left and set bit 0 assuming that
		; R >= D
		sec
		rol DIVID+3,x
		rol DIVID+2,x
		rol DIVID+1,x
		rol DIVID,x
		; N(i) is now in carry
		; R <<= 1; R(0) = N(i)
		; Capture into the working register
		rol @tmp3+1		; capturing high bit into the
		rol @tmp3		; working register bottom
		rol @tmp2+1
		rol @tmp2
		; Do a 32bit subtract but skip writing the high 16bits
		; back until we know the comparison
		;
		; R - D
		;
		ldaa @tmp3
		ldab @tmp3+1
		subb DIVIS+3,x
		sbca DIVIS+2,x
		staa @tmp3
		stab @tmp3+1
		ldaa @tmp2
		ldab @tmp2+1
		sbcb DIVIS+1,x
		sbca DIVIS,x
		; Want to subtract (R - D >= 0)
		bcc skip
		; No subtract, so put back the low 16bits we mushed
		ldaa @tmp3
		ldab @tmp3+1
		addb DIVIS+3,x
		adca DIVIS+2,x
		staa @tmp3
		stab @tmp3+1
		; We guessed the wrong way for Q(i). Clear Q(i) which is
		; in the lowest bit and we know is set so using dec is safe
		dec DIVID+3,x
done:
		dec @tmp
		bne loop
		rts
		; We do want to subtract - write back the other bits
skip:
		; R -= D
		staa @tmp2
		stab @tmp2+1
		dec @tmp
		bne loop
		rts
