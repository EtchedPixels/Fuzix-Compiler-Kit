;
;	Long complement. Possibly inlinable
;
	.export __cpll
	.setcpu 6803

	.code
__cpll:
	coma
	comb
	std @tmp
	ldd @hireg
	coma
	comb
	std @hireg
	ldd @tmp
	rts
