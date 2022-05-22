;
;	HL = lval
;
		.export __minuseq1
		.export __minuseq2

		.setcpu 8085
		.code

__minuseq1:
		xchg
		lhlx
		inx	h
		shlx
		ret

__minuseq2:
		xchg
		lhlx
		inx	h
		inx	h
		shlx
		ret
