;
;	TODO optimize
;
	.export __xshreql
	.code

__xshreql:
	clra
	andb #31
	beq load_only
shlp:
	asr 0,x
	ror 1,x
	ror 2,x
	ror 3,x
	decb
	bne shlp
load_only:
	ldd 2,x
	ldy 0,x
	rts
