;
;	Signed 16bit compare with top of stack
;
	.export __ccgt
	.export __ccgtconst
	.export __ccgtconst0
	.export __ccgtconstb
	.code

__ccgtconst0:
	clr r11
__ccgtconstb:
	clr r10
	jmp __ccgtconst
__ccgt:
	call @__pop10
__ccgtconst:
	; No direct signed comparisons so..
	xor r10,r4
	jp same_sign
	xor r10,r4
	jp true
false:
	clr r4
	clr r5
	rets
true:
	clr r5
	mov %1,r5
	rets
same_sign:
	cmp r10,r4
	jnc true
	clr r4
	clr r5
	rets
