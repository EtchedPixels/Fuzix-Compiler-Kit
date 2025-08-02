;
;	__ytmp. Put the contents of (@sp),y/y-1 into XA and the old XA
;	into @tmp

	.export __ytmp

__ytmp:
	sta @tmp
	stx @tmp+1
	lda (@sp),y
	tax
	dey
	lda (@sp),y
	rts
	