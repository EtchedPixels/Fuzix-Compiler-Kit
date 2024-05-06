	.setcpu	4

	.export __shl7
	.export __shl6
	.export __shl5
	.export __shl4
	.export __shl3

	.code

__shl7:
	slr	b
__shl6:
	slr	b
__shl5:
	slr	b
__shl4:
	slr	b
__shl3:
	slr	b
	slr	b
	slr	b
	rsr
