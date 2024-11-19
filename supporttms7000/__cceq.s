;
;	Compare top of stack ac
;
	.export __cceq
	.export __cceqconst
	.export __cceqconst0
	.export __cceqconstb
	.code

__cceq:
	call @__pop10
__cceqconst:
	cmp r10,r4
	jnz c1
	cmp r11,r5
	jz c1
	jmp ct
__cceqconst0:
	or r4,r5
c1:	clr r4
	clr r5
	rets
	jnz c1
ct:
	movd %1,r5
	rets
__cceqconstb:
	clr r10
	jmp __cceqconst
