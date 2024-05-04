
		.code
start:
		clra
		clrb
		std	@zero
		incb
		std	@one
		lbsr	_main
		pshs	d
		lbsr	_exit
