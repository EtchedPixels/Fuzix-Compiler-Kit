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
	add %0x80,r10
	add %0x80,r4
	jmp __ccltconstu
