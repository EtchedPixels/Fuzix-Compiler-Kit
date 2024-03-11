
		.code
		.setcpu 6803
start:
		jsr	_main
		ldx	#0
		pshx
		jsr	_exit
