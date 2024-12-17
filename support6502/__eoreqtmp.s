	.code

	.export __eoreqtmp
	.export __eoreqtmpu
__eoreqtmp:
__eoreqtmpu:
	ldy #0
	eor (@tmp),y
	sta (@tmp),y
	pha
	txa
	iny
	eor (@tmp),y
	sta (@tmp),y
	tax
	pla
	rts
