;
;	Signed 16bit compare with top of stack
;
	.export __ccgteq
	.export __ccgteqconst
	.export __ccgteqconst0
	.export __ccgteqconstb
	.code

__ccgteqconst0:
	clr r11
__ccgteqconstb:
	clr r10
	jmp __ccgteqconst
__ccgteq:
	call @__pop10
__ccgteqconst:
	; No direct signed comparisons so..
	add %0x80,r4
	add %0x80,r10
	jmp __ccgteqconstu
