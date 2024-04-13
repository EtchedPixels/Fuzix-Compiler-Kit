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
	ldw	gargr2,#__gargr2
	ldw	pargr2,#__pargr2
	ldw	pushln,#__pushln
	ldw	cceq,#__cceq
	ldw	ccne,#__ccne
	ldw	nref_2,#__nref_2
	ldw	nstore_2b,#__nstore_2b
	ldw	nstore_2_0,#__nstore_2_0
	ldw	nstore_2,#__nstore_2
	ldw	load2,#__load2
	ldw	garg12r2,#__garg12r2
	ldw	store2,#__store2
	ldw	nref12_2,#__nref12_2
	ret
