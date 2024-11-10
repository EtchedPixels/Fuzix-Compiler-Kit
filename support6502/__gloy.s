;
;	Get 16bit local at offset Y
;
	.export __gloy
	.export __gloy0
	.export __pushly
	.export __pushly0
	.export __gloytmp
	.export __gloytmp0

	.code
__gloy0:
	ldy #1
__gloy:
	lda (@sp),y
	tax
	dey
	lda (@sp),y
	rts

__pushly0:
	ldy #1
__pushly:
	jsr __gloy
	jmp __push

__gloytmp0:
	ldy #1
__gloytmp:
	jsr __gloy
	sta @tmp
	stx @tmp+1
	rts
