;
;	Get 16bit local at offset Y
;	CMOS oddments
;
	.export __gloy0
	.export __pushly0

	.65c02

	.code
__gloyc0:
	ldy #1
	lda (@sp),y
	tax
	lda (@sp)
	rts

__pushlyc0:
	jsr __gloyc0
	jmp __push
