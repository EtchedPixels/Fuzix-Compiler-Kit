;
;	Long complement. Possibly inlinable
;
	.export __cpll
	.code
__cpll:
	coma
	comb
	exg d,y
	coma
	comb
	exg d,y
	rts
