;
;	Signed 16bit compare with top of stack
;
	.export __cclt
	.export __ccltconst
	.export __ccltconst0
	.export __ccltconstb
	.code

__ccltconst0:
	clr r11
__ccltconstb:
	clr r10
	jmp __ccltconst
__cclt:
	call @__pop10
__ccltconst:
	; No direct signed comparisons so..
	xor r10,r4
	jpz same_sign
	xor r10,r4
	jn true
false:
	clr r4
	clr r5
	rets
true:
	clr r5
	mov %1,r5
	rets
same_sign:
	cmp r4,r10
	jnc true
	clr r4
	clr r5
	rets
