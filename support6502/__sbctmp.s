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
	; We need to calculate @tmp - XA so this is a bit messier
	; than ideal. Probably we need to inline or rework the logic
	; for the simple cases (eg eval the subtrees backwards)
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
