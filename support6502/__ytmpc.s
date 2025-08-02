;
;	__ytmpc. Put the contents of (@sp),y into A and the old A into
;	@tmp. Prob should just inline all cases of this TODO FIXME
;
	.export __ytmpc

__ytmpc:
	sta @tmp
	lda (@sp),y
	rts
	