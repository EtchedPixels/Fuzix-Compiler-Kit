;
;	Signed 16bit compare with top of stack
;
	.export __cclteq
	.export __cclteqconst
	.export __cclteqconst0
	.export __cclteqconstb
	.code

__cclteqconst0:
	clr r11
__cclteqconstb:
	clr r10
	jmp __cclteqconst
__cclteq:
	call @__pop10
__cclteqconst:
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
	clr r4
	mov %1,r5
	rets
same_sign:
	cmp r10,r4
	jc true
	clr r4
	clr r5
	rets
