	.setcpu 4
	.export __cceql

	.code
__cceql:
	; 2,s is the high 4,s the low to comare with hireg:b
	; A is free
	lda 4(s)
	sab
	bnz false
	lda 2(s)
	ldb (__hireg)
	sab
	bnz false
	ldb 1
	rsr
false:
	clr b
	rsr
