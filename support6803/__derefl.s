	.export __derefl
	.setcpu 6803
	.code

__derefl:
	std @tmp
	ldx @tmp
	ldd ,x
	std @hireg
	ldd 2,x
	rts
