;
;	Get 16bit local at offset Y
;
	.export __gloy
	.export __gloy0
	.export __pushly
	.export __pushly0

	.code
__gloy0:
	ldy #0
__gloy:
	lda (@sp),y
	tax
	dey
	lda (@sp),y
	rts

__pushly0:
	ldy #0
__pushly:
	jsr __gloy
	jmp __push

