;
;	Compare top of stack ac
;
	.export __ccne
	.export __ccneconst
	.export __ccneconst0
	.export __ccneconstb
	.code

__ccne:
	call @__pop10
__ccneconst:
	cmp r10,r4
	jnz c1
	cmp r11,r5
	jnz c1
cf:
	clr r4
	clr r5
	rets
c1:	clr r4
	mov %1,r5
	rets
__ccneconst0:
	or r4,r5
	jnz c1
	jmp cf
__ccneconstb:
	clr r10
	jmp __ccneconst
