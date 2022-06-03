
		.code
		.setcpu 8080

start:
		call	_main
		push	d
		call	_exit
