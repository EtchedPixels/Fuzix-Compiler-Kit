	.code

	.export __eor
	.export __eoru
	.export __eortmp
	.export __eortmpu
__eor:
__eoru:
	jsr __poptmp
__eortmp:
__eortmpu:
	eor @tmp
	pha
	txa
	eor @tmp+1
	tax
	pla
	rts
