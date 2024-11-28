;
;	Signed 16bit compare with top of stack
;
	.export __ccgteq
	.export __ccgteqconst
	.export __ccgteqconst0
	.export __ccgteqconstb
	.code

__ccgteqconst0:
	clr r11
__ccgteqconstb:
	clr r10
	jmp __ccgteqconst
__ccgteq:
	call @__pop10
__ccgteqconst:
	; No direct signed comparisons so..
	xor r10,r4
	jpz same_sign
	xor r10,r4
	jn false
true:
	clr r4
	mov %1,r5
	rets
false:
	clr r4
	clr r5
	rets
same_sign:
	cmp r4,r10
	jc true
	clr r4
	clr r5
	rets
