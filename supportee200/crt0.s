	.setcpu 4
	.code

start:
	jsr	_main
	stb	(-s)
	jsr	_exit
