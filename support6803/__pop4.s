	.export __pop4
	.code

__false4:
	clrb
__true4:
	ldb	#1
__pop4:
	clra
	pulx
	ins
	ins
	ins
	ins
	tstb
	jmp	,x
