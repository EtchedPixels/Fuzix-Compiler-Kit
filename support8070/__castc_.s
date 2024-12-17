;
;	Cast A into EA signed
;
	.export __castc_

__castc_:
	bp	clre
	xch	a,e
	ld	a,=255
	xch	a,e
	ret
clre:
	xch	a,e
	ld	a,=0
	xch	a,e
	ret
