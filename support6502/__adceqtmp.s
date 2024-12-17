	.code

	.export __adceqtmp
	.export __adceqtmpu
__adceqtmp:
__adceqtmpu:
	ldy #0
	clc
	adc (@tmp),y
	sta (@tmp),y
	pha
	txa
	iny
	adc (@tmp),y
	sta (@tmp),y
	tax
	pla
	rts
