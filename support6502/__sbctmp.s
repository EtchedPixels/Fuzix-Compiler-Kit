	.code

	.export __sbc
	.export __sbcu
	.export __sbctmp
	.export __sbctmpu
__sbc:
__sbcu:
	jsr __poptmp
__sbctmp:
__sbctmpu:
	sec
	sbc @tmp
	pha
	txa
	sbc @tmp+1
	tax
	pla
	rts
