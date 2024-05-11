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
	.zp

	.export __jf
	.export __jt
	.export __booljf
	.export __booljt
	.export __notjf
	.export __notjt
	.export __const1
	.export __const1l
	.export __const0
	.export __const0l
	.export __iconst1
	.export __iconst1l
	.export __iconst0
	.export __iconst0l
	.export __sconst1
	.export __sconst1l
	.export __cpush
	.export __cpushl
	.export __cipush
	.export __cipushl

	.export __derefc
	.export __derefuc
	.export __assignc
	.export __pluseqc

	.export __andeq
	.export __oreq
	.export __xoreq

	.export __castc_

	.export __condeq
	.export __condne
	.export __condltequ
	.export __condgtu
	.export __condgtequ
	.export __condltu

	.export	__cpll

	.export __divu
	.export __remu
	.export __divequ
	.export __remequ

	.export __minuseq

	.export __minusl

	.export __minuseql

	.export __mul

	.export __negl

	.export	__cpluseq
	.export	__pluseq

	.export	__cpluseql
	.export	__pluseql

	.export __postdec

	.export __postinc

	.export __shl
	.export __shru
	.export __shr
	.export __shll
	.export __shrul
	.export __shrl

__jf:		.word	f__jf
__jt:		.word	f__jt
__booljf:	.word f__booljf
__booljt:	.word f__booljt
__notjf:	.word f__notjf
__notjt:	.word f__notjt
__const1:	.word f_const1
__const1l:	.word f__const1l
__const0:	.word f__const0
__const0l:	.word f__const0l
__iconst1:	.word f__iconst1
__iconst1l:	.word f__inconst1l
__iconst0:	.word f__iconst1
__iconst0l:	.word f__inconst1l
__sconst1:	.word f__sconst1
__sconst1l:	.word f__sconst1l
__cpush:	.word f__cpush
__cpushl:	.word f__cpushl
__cipush:	.word f__cipush
__cipushl:	.word f__cipushl

__derefc:
__derefuc:	.word f__derefc
__assignc:	.word f__assignc
__pluseqc:	.word f__pluseqc

__andeq:	.word f__andeq
__oreq:		.word f__oreq
__xoreq:	.word f__xoreq

__castc_:	.word f__castc_

__condeq:	.word f__condeq
__condne:	.word f__condne
__condltequ:	.word f__condltequ
__condgtu:	.word f__condgtu
__condgtequ:	.word f__confgtequ
__condltu:	.word f__condltu

__cpll:		.word f__cpll

__divu:		.word f__divu
__remu:		.word f__remu
__divequ:	.word f__divequ
__remequ:	.word f__remequ

__minuseq:	.word f__minuseq

__minusl:	.word f__minusl

__minuseql:	.word f__minuseql

__mul:		.word f__mul

__negl:		.word f__negl

__cpluseq:	.word f__cpluseq
__pluseq:	.word f__pluseq

__cpluseql:	.word f__cpluseql
__pluseql:	.word f__pluseql

__postdec:	.word f__postdec

__postinc:	.word f__postinc

__shl:		.word f__shrl
__shru:		.word f__shru
__shr:		.word f__shr
__shll:		.word f__shll
__shrul:	.word f__shrul
__shrl:		.word f__shrl
