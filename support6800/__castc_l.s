	.export __castc_l
	.export __castc_ul

__castc_l:
__castc_ul:
	clra
	asrb
	rolb
	sbca #$00
	sta @hireg
	sta @hireg+1
	rts
