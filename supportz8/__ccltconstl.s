;
;	r12-r15 v r0-r3
;
;	TODO: ponder instead putting the constant as the words following ?
;
	.export __ccltconstl
	.export __ccltconstul
	.export __ccgteqconstl
	.export __ccgteqconstul

;	r12-r15 > r0-r3 ?

__ccltconstl:
	cp r12,r0
	jr lt,true
	jr nz,false
	jr next

__ccltconstul:
	cp r12,r0
	jr ult,true
	jr nz,false
next:
	cp r13,r1
	jr ult,true
	jr nz,false
	cp r14,r2
	jr ult,true
	jr nz,false
	cp r15,r3
	jr ult,true
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

__ccgteqconstl:
	call __ccltconstl
	xor r3,#1
	ret

__ccgteqconstul:
	call __ccltconstul
	xor r3,#1
	ret
