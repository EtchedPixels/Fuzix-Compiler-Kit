;
;	Direct page
;
	.65c816
	.a16
	.i16

	.export tmp
	.export tmp2
	.export tmp3
	.export hireg
	.export divisor
	.export dividend
	.export sum
	.export sign

	.zp

tmp:	.word 0
hireg:	.word 0
tmp2:
divisor:
sum:	.word 0
tmp3:
dividend:
	.word 0
sign:
	.word 0
