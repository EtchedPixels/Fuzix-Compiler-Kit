	movd %0xefff,r15
	mov %0x20,b
	ldsp
	call @_main
	mov r5,a
	movp a,p255

