;
;	r12-r15 v r0-r3
;
;	TODO: ponder instead putting the constant as the words following ?
;
	.export __ccgtconstl
	.export __ccgtconstul
	.export __cclteqconstl
	.export __cclteqconstul
	.export __ccgtconstbl
	.export __ccgtconstbul
	.export __cclteqconstbl
	.export __cclteqconstbul
	.export __ccgtconst0l
	.export __ccgtconst0ul
	.export __cclteqconst0l
	.export __cclteqconst0ul

;	r10-r13 > r2-r5 ?

__ccgtconst0ul:
	clr r13
__ccgtconstbul:
	clr r12
	clr r11
	clr r10
	jmp __ccgtconstul
__ccgtconst0l:
	clr r13
__ccgtconstbl:
	clr r12
	clr r11
	clr r10
__ccgtconstl:
	cmp r10,r2
;FIXME	jr gt,true
	jnz false
	jmp next

__ccgtconstul:
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
	clr r5
	clr r4
	rets
true:
	clr r4
	mov %1,r5
	rets

__cclteqconst0l:
	clr r13
__cclteqconstbl:
	clr r12
	clr r11
	clr r10
__cclteqconstl:
	call @__ccgtconstl
	xor %1,r5
	rets

__cclteqconst0ul:
	clr r13
__cclteqconstbul:
	clr r12
	clr r11
	clr r10
__cclteqconstul:
	call @__ccgtconstul
	xor %1,r3
	rets
