	.export __muleq
	.export __mulequ

	.code

__muleq:
__mulequ:
	ld	t,ea
	ld	ea,2,p1		; ptr
	ld	p3,ea
	ld	ea,0,p3
	jsr	mpyfix
	st	ea,0,p3
	ret

	