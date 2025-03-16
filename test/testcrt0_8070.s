;
;	Stub for running tests
;
	.code
	ld p1,=0x8000
	ld p3,=_main
	jsr _main
	ld p1,#0xFEFF
	st a,0,p1

	.export __tmp
	.export __hireg

	.dp
__tmp:
	.word 0
__hireg:
	.word 0
