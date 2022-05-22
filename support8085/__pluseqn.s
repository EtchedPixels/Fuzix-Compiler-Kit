;
;	HL = lval
;
		.export __pluseq1
		.export __pluseq2

		.setcpu 8085
		.code

__pluseq1:
		xchg
		lhlx
		inx	h
		shlx
		ret

__pluseq2:
		xchg
		lhlx
		inx	h
		inx	h
		shlx
		ret
