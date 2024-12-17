	.code

	.export __oraeqtmp
	.export __oraeqtmpu
__oraeqtmp:
__oraeqtmpu:
	ldy #0
	ora (@tmp),y
	sta (@tmp),y
	pha
	txa
	iny
	ora (@tmp),y
	sta (@tmp),y
	tax
	pla
	rts
