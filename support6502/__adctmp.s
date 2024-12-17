	.code

	.export __adc
	.export __adcu
	.export __adctmp
	.export __adctmpu
__adc:
__adcu:
	jsr __poptmp
__adctmp:
__adctmpu:
	clc
	adc @tmp
	pha
	txa
	adc @tmp+1
	tax
	pla
	rts
