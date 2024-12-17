	.code

	.export __and
	.export __andu
	.export __andtmp
	.export __andtmpu
__and:
__andu:
	jsr __poptmp
__andtmp:
__andtmpu:
	and @tmp
	pha
	txa
	and @tmp+1
	tax
	pla
	rts
