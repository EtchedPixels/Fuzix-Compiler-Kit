;
;	 Left shift TOS.L by D
;
;	TODO optimize 8/16/24 bits of shift with moves first
;
	.export __shrl
	.code

__shrl:	clra
	ldx ,s
	andb #31
	beq load_out
shrnext:
	asr 2,s
	ror 3,s
	ror 4,s
	ror 5,s
	decb
	bne shrnext
load_out:
	ldd 4,s
	ldy 2,s
	leas 6,s
	jmp ,x
