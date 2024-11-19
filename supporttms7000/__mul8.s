	.export __mul8
	.export __mul4

	.code

__mul8:
	add r3,r3
	adc r2,r2
__mul4:
	add r3,r3
	adc r2,r2
	add r3,r3
	adc r2,r2
	rets

