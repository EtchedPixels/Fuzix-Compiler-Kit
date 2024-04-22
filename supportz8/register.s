;
;	Optimizer rewrites of common calls to register ones
;
	.zp

	.export gargr2
	.export pargr2
	.export pushln
	.export cceq
	.export ccne
	.export nref_2
	.export nstore_2b
	.export nstore_2_0
	.export nstore_2
	.export load2
	.export garg12r2
	.export store2
	.export nref12_2

gargr2:
	.word	0
pargr2:
	.word	0
pushln:
	.word	0
cceq:
	.word	0
ccne:
	.word	0
nref_2:
	.word	0
nstore_2b:
	.word	0
nstore_2_0:
	.word	0
nstore_2:
	.word	0
load2:
	.word	0
garg12r2:
	.word	0
store2:
	.word	0
nref12_2:
	.word	0

	.code

	.export __reginit

__reginit:
	ld	gargr2,#>__gargr2
	ld	gargr2+1,#<__gargr2
	ld	pargr2,#>__pargr2
	ld	pargr2+1,#<__pargr2
	ld	pushln,#>__pushln
	ld	pushln,#<__pushln
	ld	cceq,#>__cceq
	ld	cceq+1,#<__cceq
	ld	ccne,#>__ccne
	ld	ccne+1,#<__ccne
	ld	nref_2,#>__nref_2
	ld	nref_2+1,#<__nref_2
	ld	nstore_2b,#>__nstore_2b
	ld	nstore_2b+1,#<__nstore_2b
	ld	nstore_2_0,#>__nstore_2_0
	ld	nstore_2_0+1,#<__nstore_2_0
	ld	nstore_2,#>__nstore_2
	ld	nstore_2+1,#<__nstore_2
	ld	load2,#>__load2
	ld	load2+1,#<__load2
	ld	garg12r2,#>__garg12r2
	ld	garg12r2+1,#<__garg12r2
	ld	store2,#>__store2
	ld	store2+1,#<__store2
	ld	nref12_2,#>__nref12_2
	ld	nref12_2+1,#<__nref12_2
	ret
