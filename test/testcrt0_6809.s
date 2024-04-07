	.dp

	.export hireg
	.export tmp
	.export zero
	.export	one

hireg:	.word	0
tmp:	.word	0
zero:	.word	0
one:	.word	1

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
