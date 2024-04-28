
	.export __regand
	.export __regor
	.export __regeor

	.code
__regand:
	pshs u
	anda ,s+
	andb ,s+
	tfr d,u
	rts

__regor:
	pshs u
	ora ,s+
	orb ,s+
	tfr d,u
	rts

__regeor:
	pshs u
	eora ,s+
	eorb ,s+
	tfr d,u
	rts
