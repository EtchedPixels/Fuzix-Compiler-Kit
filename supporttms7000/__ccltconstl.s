;
;	r10-r13 v r2-r5
;
;	TODO: ponder instead putting the constant as the words following ?
;
	.export __ccltconstl
	.export __ccltconstul
	.export __ccgteqconstl
	.export __ccgteqconstul
	.export __ccltconstbl
	.export __ccltconstbul
	.export __ccgteqconstbl
	.export __ccgteqconstbul
	.export __ccltconst0l
	.export __ccltconst0ul
	.export __ccgteqconst0l
	.export __ccgteqconst0ul

;	r12-r15 > r0-r3 ?

__ccltconst0ul:
	clr r13
__ccltconstbul:
	clr r12
	clr r11
	clr r10
	jmp __ccltconstul
__ccltconst0l:
	clr r13
__ccltconstbl:
	clr r12
	clr r11
	clr r10
__ccltconstl:
	xor r10,r2
	jpz samesign
	xor r10,r2
	jpz true
	jmp false
samesign:
	xor r10,r2
	cmp r10,r2
	jc true
	jnz false
	jmp next

__ccltconstul:
	cmp r10,r2
	jc true
	jnz false
next:
	cmp r11,r3
	jc true
	jnz false
	cmp r12,r4
	jc true
	jnz false
	cmp r13,r5
	jc true
false:
	clr r4
	clr r5
	rets
true:
	clr r4
	mov %1,r5
	rets

__ccgteqconst0l:
	clr r13
__ccgteqconstbl:
	clr r12
	clr r11
	clr r10
__ccgteqconstl:
	call @__ccltconstl
	xor %1,r5
	rets

__ccgteqconst0ul:
	clr r13
__ccgteqconstbul:
	clr r12
	clr r11
	clr r10
__ccgteqconstul:
	call @__ccltconstul
	xor %1,r5
	rets
