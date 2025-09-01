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

	.export __enter
	.export __ret

	.export __derefc
	.export __derefuc
	.export __assignc
	.export __pluseqc
	.export __eqcget
	.export __equcget

	.export __andeq
	.export __oreq
	.export __xoreq

	.export __andeql
	.export __oreql
	.export __xoreql
	.export __andl
	.export __orl
	.export __xorl

	.export __castc_

	.export __condeq
	.export __condne
	.export __condltequ
	.export __condgtu
	.export __condgtequ
	.export __condltu
	.export __condlteq
	.export __condgt
	.export __condgteq
	.export __condlt

	.export __cceql
	.export __ccnel
	.export __ccltul
	.export __ccltequl
	.export __ccgtul
	.export __ccgtequl
	.export __ccltl
	.export __cclteql
	.export __ccgtl
	.export __ccgteql

	.export	__cpll

	.export __divu
	.export __remu
	.export __divequ
	.export __remequ

	.export __div
	.export __rem
	.export __diveq
	.export __remeq

	.export __divul
	.export __remul
	.export __divequl
	.export __remequl

	.export __divl
	.export __reml
	.export __diveql
	.export __remeql

	.export __minuseq

	.export __minusl

	.export __minuseql

	.export __mul

	.export __mull

	.export __negl

	.export	__cpluseq
	.export	__pluseq

	.export	__cpluseql
	.export	__pluseql

	.export __postdec

	.export __postinc
	.export __postincl

	.export __shl
	.export __shru
	.export __shr
	.export __shll
	.export __shrul
	.export __shrl

	.export __shleq
	.export __shrequ
	.export __shreq
	.export __shleql
	.export __shrequl
	.export __shreql

	.export __switchc
	.export __switch
	.export __switchl

__jf:		.word	f__jf
__jt:		.word	f__jt
__booljf:	.word f__booljf
__booljt:	.word f__booljt
__notjf:	.word f__notjf
__notjt:	.word f__notjt
__const1:	.word f__const1
__const1l:	.word f__const1l
__const0:	.word f__const0
__const0l:	.word f__const0l
__iconst1:	.word f__iconst1
__iconst1l:	.word f__iconst1l
__iconst0:	.word f__iconst0
__iconst0l:	.word f__iconst0l
__sconst1:	.word f__sconst1
__sconst1l:	.word f__sconst1l
__cpush:	.word f__cpush
__cpushl:	.word f__cpushl
__cipush:	.word f__cipush
__cipushl:	.word f__cipushl

__enter:	.word f__enter
__ret:		.word f__ret

__derefc:	.word f__derefc
__derefuc:	.word f__derefuc
__assignc:	.word f__assignc
__pluseqc:	.word f__pluseqc
__eqcget:	.word f__eqcget
__equcget:	.word f__equcget

__andeq:	.word f__andeq
__oreq:		.word f__oreq
__xoreq:	.word f__xoreq

__andeql:	.word f__andeql
__oreql:	.word f__oreql
__xoreql:	.word f__xoreql
__andl:		.word f__andl
__orl:		.word f__orl
__xorl:		.word f__xorl

__castc_:	.word f__castc_

__condeq:	.word f__condeq
__condne:	.word f__condne
__condltequ:	.word f__condltequ
__condgtu:	.word f__condgtu
__condgtequ:	.word f__condgtequ
__condltu:	.word f__condltu
__condlteq:	.word f__condlteq
__condgt:	.word f__condgt
__condgteq:	.word f__condgteq
__condlt:	.word f__condlt

__cceql:	.word f__cceql
__ccnel:	.word f__ccnel
__ccltul:	.word f__ccltul
__ccltequl:	.word f__ccltequl
__ccgtul:	.word f__ccgtul
__ccgtequl:	.word f__ccgtequl
__ccltl:	.word f__ccltl
__cclteql:	.word f__cclteql
__ccgtl:	.word f__ccgtl
__ccgteql:	.word f__ccgteql

__cpll:		.word f__cpll

__divu:		.word f__divu
__remu:		.word f__remu
__divequ:	.word f__divequ
__remequ:	.word f__remequ

__div:		.word f__div
__rem:		.word f__rem
__diveq:	.word f__diveq
__remeq:	.word f__remeq

__divul:	.word f__divul
__remul:	.word f__remul
__divequl:	.word f__divequl
__remequl:	.word f__remequl

__divl:		.word f__divl
__reml:		.word f__reml
__diveql:	.word f__diveql
__remeql:	.word f__remeql

__minuseq:	.word f__minuseq

__minusl:	.word f__minusl

__minuseql:	.word f__minuseql

__mul:		.word f__mul

__mull:		.word f__mull

__negl:		.word f__negl

__cpluseq:	.word f__cpluseq
__pluseq:	.word f__pluseq

__cpluseql:	.word f__cpluseql
__pluseql:	.word f__pluseql

__postdec:	.word f__postdec

__postinc:	.word f__postinc
__postincl:	.word f__postincl

__shl:		.word f__shl
__shru:		.word f__shru
__shr:		.word f__shr
__shll:		.word f__shll
__shrul:	.word f__shrul
__shrl:		.word f__shrl

__shleq:	.word f__shleq
__shrequ:	.word f__shrequ
__shreq:	.word f__shreq
__shleql:	.word f__shleql
__shrequl:	.word f__shrequl
__shreql:	.word f__shreql

__switchc:	.word f__switchc
__switch:	.word f__switch
__switchl:	.word f__switchl
