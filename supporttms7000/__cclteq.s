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
	add %0x80,r4
	add %0x80,r10
	jmp __cclteqconstu
