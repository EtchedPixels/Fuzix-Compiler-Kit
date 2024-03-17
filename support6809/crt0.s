
		.code
start:
		jsr	_main
		ldx	#0
		stx	,--s
		jsr	_exit
