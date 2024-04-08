	.dp

	.export hireg
	.export zero
	.export	one

	.export hireg
	.export zero
	.export one

hireg:	.word	0
zero:	.byte	0	; overlaps 1
one:	.word	0

	.export tmp
	.export tmp2
	.export tmp3
	.export tmp4

tmp:	.word	0
tmp2:	.word	0
tmp3:	.word	0
tmp4:	.word	0


	.code ; (at 0x0100)

start:
	ldd	#0
	std	@zero
	ldd	#1
	std	@one
	jsr	_main
	; return and exit (value is in XA)
	stb	$FEFF
	; Write to FEFF terminates

	.export _printint
_printint:
	ldd	2,s
	std	$FEFC
	rts

	.export _printchar
_printchar:
	ldd	2,s
	stb	$FEFE
	rts
