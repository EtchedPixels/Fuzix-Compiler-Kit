
		.code
start:
		ldd	#0
		std	@zero
		ldd	#1
		std	@one
		jsr	_main
		ldx	#0
		stx	,--s
		jsr	_exit
