		.export __bandde
		.code
		.setcpu 8080

__bandde:
	mov a,d
	ana h
	mov h,a
	mov a,e
	ana l
	mov l,a
	ret
