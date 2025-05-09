;
;	Signed 16bit compare with top of stack
;
	.export __ccgt
	.export __ccgtconst
	.export __ccgtconst0
	.export __ccgtconstb
	.code

__ccgtconst0:
	clr r11
__ccgtconstb:
	clr r10
	jmp __ccgtconst
__ccgt:
	call @__pop10
__ccgtconst:
	; No direct signed comparisons so..
	add %0x80,r4
	add %0x80,r10
	jmp __ccgtconstu
