	.export f__castc_

f__castc_:
	sta	3,__tmp,0
	lda	0,N255,0
	and	0,1		; mask byte
	com	0,0		; now FF00
	movs	1,2		; char byte high copy so we can check sign
	movl	2,2,szc		; We are good if clear
	add	0,1		; extend sign
	lda	3,__fp,0
	jmp	@__tmp,0
