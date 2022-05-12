
		.code
		.setcpu 8085

start:
		call	_main
		push	d
		call	_exit
