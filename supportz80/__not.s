		.export	__not
		.export	__cctonot
		.code

__not:
		ld	a,h
		or	l
__cctonot:
		jp	z,__true
		jp	__false
