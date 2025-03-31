	.export __pluseqc

;	(TOS.B) += A

__pluseqc:
	pop	p2
	pop	p3
	push	p2
	add	a,0,p3
	st	a,0,p3
	ret
