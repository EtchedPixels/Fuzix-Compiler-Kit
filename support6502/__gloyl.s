;
;	Get 32bit local at offset Y
;
	.export __gloyl
	.export __gloy0l
	.export __pushlyl
	.export __pushly0l
;	.export __gloytmpl
;	.export __gloytmp0l

	.code
__gloy0l:
	ldy #3
__gloyl:
	lda (@sp),y
	sta @hireg+1
	dey
	lda (@sp),y
	sta @hireg
	dey
	lda (@sp),y
	tax
	dey
	lda (@sp),y
	rts

__pushly0l:
	ldy #3
__pushlyl:
	jsr __gloyl
	jmp __pushl

;__gloytmp0l:
;	ldy #3
;__gloytmpl:
;	lda (@sp),y
;	sta @tmp+3
;	dey
;	lda (@sp),y
;	sta @tmp+2
;	dey
;	ldx (@sp),y
;	stx @tmp+1
;	dey
;	lda (@sp),y
;	sta @tmp
;	rts
