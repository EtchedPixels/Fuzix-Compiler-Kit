	.export __derefl
	.setcpu 6303
	.code

__derefl:
	xgdx
	ldd ,x
	std @hireg
	ldd 2,x
	rts
