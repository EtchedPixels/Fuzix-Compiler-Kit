	.export __castc_l
	.export __castc_ul

__castc_l:
__castc_ul:
	clra
	bitb #$80
	beq ispve
	deca
ispve:
	sta @hireg
	sta @hireg+1
	rts
