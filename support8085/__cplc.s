		.export	__negatec
		.export __cplc

		.setcpu 8085
		.code

__negatec:
		dcr	l
__cplc:
		mov	a,l
		cma
		mov	l,a
		ret
