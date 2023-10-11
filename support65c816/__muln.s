	.65c816
	.a16
	.i16

	.export __mul16
	.export __mul32
	.export __mul64
	.export __mul128

__mul128:
	asl a
__mul64:
	asl a
__mul32:
	asl a
__mul16:
	asl a
	asl a
	asl a
	asl a
	rts
