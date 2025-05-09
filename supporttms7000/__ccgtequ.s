;
;	Compare top of stack ac
;
	.export __ccgtequ
	.export __ccgteqconstu
	.export __ccgteqconst0u
	.export __ccgteqconstbu
	.code

__ccgteqconst0u:
	clr r11
__ccgteqconstbu:
	clr r10
	jmp __ccgteqconstu
__ccgtequ:
	call @__pop10
__ccgteqconstu:
	cmp r10,r4
	jnz c1
	cmp r11,r5
	jz c2
c1:
	jnc c2
	clr r4
	clr r5
	rets
c2:
	clr r4
	mov %1,r5
	rets

