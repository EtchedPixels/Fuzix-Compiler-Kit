;
;	Compare top of stack ac
;
	.export __ccltequ
	.export __cclteqconstu
	.export __cclteqconst0u
	.export __cclteqconstbu
	.code

__cclteqconst0u:
	clr r11
__cclteqconstbu:
	clr r10
	jmp __cclteqconstu
__ccltequ:
	call @__pop10
__cclteqconstu:
	cmp r10,r4
	jnz c1
	cmp r11,r5
	jz true
c1:
	jc true
c2:
	clr r4
	clr r5
	rets
true:
	clr r4
	mov %1,r5
	rets
