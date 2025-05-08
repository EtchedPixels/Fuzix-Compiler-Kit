	.export __muleqtmp
	.export __muleqtmpu

	.code

__muleqtmp:
__muleqtmpu:
	; tmp is the value, ea the pointer
	xch	ea,p2
	ld	ea,0,p2
	ld	t,:__tmp
	jsr	__mpyfix
	st	ea,0,p2
	ret

	