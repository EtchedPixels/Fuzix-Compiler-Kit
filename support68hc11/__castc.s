	.export __castc_
	.export __castc_u

__castc_:
__castc_u:
	clra
	bitb #$80
	beq ispve
	deca
ispve:
	rts
