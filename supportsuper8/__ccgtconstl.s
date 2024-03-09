;
;	r12-r15 v r0-r3
;
;	TODO: ponder instead putting the constant as the words following ?
;
	.export __ccgtconstl
	.export __ccgtconstul
	.export __cclteqconstl
	.export __cclteqconstul
	.export __ccgtconstl0
	.export __ccgtconstul0
	.export __cclteqconstl0
	.export __cclteqconstul0

;	r12-r15 > r0-r3 ?

__ccgtconstul0:
	clr r12
	clr r13
	clr r14
	clr r15
	jr __ccgtconstul
__ccgtconstl0:
	clr r12
	clr r13
	clr r14
	clr r15
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

__cclteqconstl0:
	clr r12
	clr r13
	clr r14
	clr r15
__cclteqconstl:
	call __ccgtconstl
	xor r3,#1
	ret

__cclteqconstul0:
	clr r12
	clr r13
	clr r14
	clr r15
__cclteqconstul:
	call __ccgtconstul
	xor r3,#1
	ret
