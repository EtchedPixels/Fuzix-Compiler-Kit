	.code ; (at 0x0100)

	.setcpu	6803
start:
	clr	@zero
	ldd	#1
	std	@one
	jsr	_main
	; return and exit (value is in XA)
	stab	$FEFF
	; Write to FEFF terminates
