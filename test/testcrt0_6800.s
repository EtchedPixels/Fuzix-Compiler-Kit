	.setcpu 6800
;	.zp
;	.export hireg
;	.export zero
;	.export	one
;
;hireg:	.word	0
;zero:	.byte	0	; overlaps 1
;one:	.word	0
;
;	.export tmp
;	.export tmp2
;	.export tmp3
;	.export tmp4
;	.export tmp5
;
;tmp:	.word	0
;tmp2:	.word	0
;tmp3:	.word	0
;tmp4:	.word	0
;tmp5:	.word	0


	.code ; (at 0x0100)

start:
	lds	#$7FFF
	clrb
	clra
	stab @zero+1
	staa @zero
	incb
	stab @one+1
	staa @one
	psha	; dummy argc
	pshb
	psha	; dummy argv
	pshb
	jsr	_main
	; return and exit (value is in XA)
	stab	$FEFF
	; Write to FEFF terminates

	.export _printint
_printint:
	tsx
	ldab 3,x
	ldaa 2,x
	stab $fefc+1
	staa $fefc
	ldx 0,x
	ins
	ins
	ins
	ins
	jmp 0,x

	.export _printchar
_printchar:
	tsx
	ldab 3,x
	stab $FEFE
	ldx 0,x
	ins
	ins
	ins
	ins
	jmp 0,x
