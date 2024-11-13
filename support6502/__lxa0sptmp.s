;
;	Very common pattern used as a straight
;	peephole. Only generated and thus only
;	replaced on 65C02
;
	.export __lxa0sptmp

	.code
	.65c02

; This routine 
__lxa0sptmp:
	lda (@sp),y
	tax
	lda (@sp)
	sta @tmp
	stx @tmp+1
	rts
