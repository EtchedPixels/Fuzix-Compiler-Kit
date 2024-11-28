;
;	Compare top of stack ac
;
	.export __ccltu
	.export __ccltconstu
	.export __ccltconst0u
	.export __ccltconstbu
	.code

__ccltconst0u:
	clr r11
__ccltconstbu:
	clr r10
	jmp __ccltconstu
__ccltu:
	call @__pop10
__ccltconstu:
	cmp r10,r4
	jnz c1
	cmp r11,r5
c1:
	; Annoyingly C is affected by mov or clr
	; if C set then <
	jnc c2
	clr r4
	mov %1,r5
	rets
c2:
	clr r4
	clr r5
	rets

