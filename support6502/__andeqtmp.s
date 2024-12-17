	.code

	.export __andeqtmp
	.export __andeqtmpu
__andeqtmp:
__andeqtmpu:
	ldy #0
	and (@tmp),y
	sta (@tmp),y
	pha
	txa
	iny
	and (@tmp),y
	sta (@tmp),y
	tax
	pla
	rts
