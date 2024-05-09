;
;	Our fixed page 0 stuff
;
	.zp
N255:	.word	255
__tmp:	.word	0
__hireg:.word	0

;
;	Helper functions. We could do this two ways.
;
;	1. Declare each .zp entry with the function
;	2. Declare them all here
;
;	2 means we build a single runtime support that has everything in it
;	but can be shared, 1 means we build just the routines needed but
;	the lib cannot be shared. Not clear which is optimal
;
	.export __jf
	.export __jt
	.export __booljf
	.export __booljt
	.export __notjf
	.export __notjt
	.export __const
	.export __constl
	.export __const0
	.export __const0l
	.export __iconst
	.export __iconstl
	.export __sconst
	.export __sconstl
	.export __cpush
	.export __cpushl

	.export __derefc
	.export __derefuc
	.export __assignc

	.export __condeq
	.export __condne
	.export __condltequ
	.export __condgtu
	.export __condgtequ
	.export __condltu

	.export	__cpll

	.export __divu

	.export __minuseq

	.export __minusl

	.export __mul

	.export __negl

	.export	__pluseq

	.export __postdec

	.export __postinc

__jf:		.word	f__jf
__jt:		.word	f__jt
__booljf:	.word f__booljf
__booljt:	.word f__booljt
__notjf:	.word f__notjf
__notjt:	.word f__notjt
__const:	.word f_const
__constl:	.word f__constl
__const0:	.word f__const0
__const0l:	.word f__const0l
__iconst:	.word f__iconst
__iconstl:	.word f__inconstl
__sconst:	.word f__sconst
__sconstl:	.word f__sconstl
__cpush:	.word f__cpush
__cpushl:	.word f__cpushl

__derefc:
__derefuc:	.word f__derefc
__assignc:	.word f__assignc

__condeq:	.word f__condeq
__condne:	.word f__condne
__condltequ:	.word f__condltequ
__condgtu:	.word f__condgtu
__condgtequ:	.word f__confgtequ
__condltu:	.word f__condltu

__cpll:		.word f__cpll

__divu:		.word f__divu

__minuseq:	.word f__minuseq

__minusl:	.word f__minusl

__mul:		.word f__mul

__negl:		.word f__negl

__pluseq:	.word f__pluseq

__postdec:	.word f__postdec

__postinc:	.word f__postinc

