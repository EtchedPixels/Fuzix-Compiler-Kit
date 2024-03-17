	.dp

	.export hireg
	.export tmp
	.export zero
	.export	one

hireg:	.word	0
tmp:	.word	0
zero:	.byte	0
one:	.word	1

	.code ; (at 0x0100)

start:
	clr	@zero
	ldd	#1
	std	@one
	jsr	_main
	; return and exit (value is in XA)
	stb	$FEFF
	; Write to FEFF terminates
