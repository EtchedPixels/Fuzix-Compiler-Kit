		.export	__notc
		.code

__notc:
		ld	a,l
		ora	a
		jp	z,__true
		jp	__false
