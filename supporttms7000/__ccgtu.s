;
;	Compare top of stack ac
;
	.export __ccgtu
	.export __ccgtconstu
	.export __ccgtconst0u
	.export __ccgtconstbu
	.code

__ccgtconst0u:
	clr r11
__ccgtconstbu:
	clr r10
	jmp __ccgtconstu
__ccgtu:
	call @__pop10
__ccgtconstu:
	cmp r10,r4
	jnz c1
	cmp r11,r5
	jz false
c1:
	; if C set then >
	jnc false
	clr r4
	mov %1,r5
	rets
false:
	clr r4
	clr r5
	rets
