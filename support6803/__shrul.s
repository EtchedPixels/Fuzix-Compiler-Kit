;
;	 Left shift TOS.L by D
;
;	TODO optimize 8/16/24 bits of shift with moves first
;
	.export __shrul
	.code

	.setcpu 6803

__shrul:clra
	tsx
	andb #31
	beq load_out
shrnext:
	lsr 2,x
	ror 3,x
	ror 4,x
	ror 5,x
	decb
	bne shrnext
load_out:
	pulx
	pula
	pulb
	std @hireg
	pula
	pulb
	jmp ,x
