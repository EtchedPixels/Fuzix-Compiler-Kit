
	.code

start:
	call	@_main
	; TODO; this will be right once we fix first arg in regs
	call	@_exit
