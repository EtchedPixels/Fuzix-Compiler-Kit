
	.export _regand
	.export _regor
	.export _regeor

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

__regxor:
	pshs u
	eora ,s+
	eorb ,s+
	tfr d,u
	rts
