	.setcpu 4
	.export __bool
	.export __boolc
	.export __not
	.export __notc

	.export __cceq
	.export __ccne
	.export	__ccgt
	.export __ccgteq
	.export __ccgtu
	.export __ccgtequ
	.export __cclt
	.export __cclteq
	.export __ccltu
	.export __ccltequ

	.code

	; 0 is false
__boolc:
	clrb	bh
__bool:
	; Boolify a word
	ori	b,b
__ccne:
	bz	ret0
ret1:
	ldb	1
	rsr

	; 0 is true
__notc:
	clrb	bh
__not:
	ori	b,b
__cceq:
	bnz	ret1
ret0:
	clr	b
	rsr

__cclt:
	bz	ret0
__cclteq:
	ble	ret1
	bra	ret0

__ccltu:
	bz	ret0
__ccltequ:
	bnl	ret1
	bra	ret0

__ccgteq:
	bz	ret1
__ccgt:
	ble	ret0
	bra	ret1

__ccgtequ:
	bz	ret1
__ccgtu:
	bnl	ret0
	bra	ret1
