	.export __boolc
	.export __bool
	.export __notc
	.export __not

	.code

__bool:
	or a,e
	ld e,a		; if 0 then now all 0
__boolc:
	bz doret
true:
	ld ea,=1
doret:
	ret

__not:
	or a,e
__notc:
	bz true
	ld ea,=0
	ret
