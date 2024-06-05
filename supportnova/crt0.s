	.code

__main:
	jsr	@1,1
	.word	_main
	sta	1,__sp,0
	jsr	@1,1
;	.word	_exit
