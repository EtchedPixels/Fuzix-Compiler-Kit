;
;	r12-r15 v r0-r3
;
;	TODO: ponder instead putting the constant as the words following ?
;
	.export __ccgtconstl
	.export __ccgtconstul
	.export __cclteqconstl
	.export __cclteqconstul
	.export __ccgtconst0l
	.export __ccgtconstbl
	.export __ccgtconst0ul
	.export __ccgtconstbul
	.export __cclteqconst0l
	.export __cclteqconstbl
	.export __cclteqconst0ul
	.export __cclteqconstbul

;	r12-r15 > r0-r3 ?

__ccgtconst0ul:
	clr r15
__ccgtconstbul:
	clr r14
	clr r13
	clr r12
	jr __ccgtconstul
__ccgtconst0l:
	clr r15
__ccgtconstbl:
	clr r14
	clr r13
	clr r12
__ccgtconstl:
	cp r12,r0
	jr gt,true
	jr nz,false
	jr next

__ccgtconstul:
	cp r12,r0
	jr ugt,true
	jr nz,false
next:
	cp r13,r1
	jr ugt,true
	jr nz,false
	cp r14,r2
	jr ugt,true
	jr nz,false
	cp r15,r3
	jr ugt,true
false:
	clr r3
	clr r2
	or r3,r3
	ret
true:
	ld r3,#1
	clr r2
	or r3,r3
	ret

__cclteqconst0l:
	clr r15
__cclteqconstbl:
	clr r14
	clr r13
	clr r12
__cclteqconstl:
	call __ccgtconstl
	xor r3,#1
	ret

__cclteqconst0ul:
	clr r15
__cclteqconstbul:
	clr r14
	clr r13
	clr r12
__cclteqconstul:
	call __ccgtconstul
	xor r3,#1
	ret
