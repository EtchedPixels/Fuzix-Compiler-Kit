;
;	Our fixed page 0 stuff
;
	.zp

	.export N255
	.export __hireg
	.export __tmp
	.export __tmp2
	.export __tmp3
	.export __tmp4
	.export __tmp5
	.export __fp
	.export __sp

N255:	.word	255
__fp:	.word	0
__hireg:.word	0
__tmp:	.word	0
__tmp2: .word	0
__tmp3:	.word	0
__tmp4:	.word	0
__tmp5:	.word	0
__tmp6:	.word	0

;
;	Autoinc for SP
;
__sp	.equ	020

