	.code

;
;	Could do with cleaning up
;
	.export __sbc
	.export __sbcu
	.export __sbctmp
	.export __sbctmpu
__sbc:
__sbcu:
	jsr __poptmp
__sbctmp:
__sbctmpu:
	sta @tmp2
	stx @tmp2+1
	sec
	lda @tmp
	sbc @tmp2
	pha
	lda @tmp+1
	sbc @tmp2+1
	tax
	pla
	rts
