	.setcpu	4
	.code

	.export __castc_
	.export __castc_u

__castc_:
__castc_u:
	clrb	bh
	orib	bl,bl
	bp	done
	dcrb	bh
done:	rsr

	