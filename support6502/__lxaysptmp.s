;
;	Very common pattern used as a straight
;	peephole
;
	.export __lxaysptmp

	.code

__lxaysptmp:
	lda (@sp),y
	tax
	dey
	lda (@sp),y
	sta @tmp
	stx @tmp+1
	rts
