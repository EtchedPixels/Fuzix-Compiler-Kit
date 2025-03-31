	.export __muleq
	.export __mulequ

	.code

	; FIXME deal with full 16x16 bit range rules

__muleq:
__mulequ:
	ld	t,ea
	ld	ea,2,p1		; ptr
	ld	p3,ea
	ld	ea,0,p3
	mpy	ea,t
	ld	ea,t
	st	ea,0,p3
	ret

	