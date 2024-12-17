	.code

	.export __ora
	.export __orau
	.export __oratmp
	.export __oratmpu
__ora:
__orau:
	jsr __poptmp
__oratmp:
__oratmpu:
	ora @tmp
	pha
	txa
	ora @tmp+1
	tax
	pla
	rts
