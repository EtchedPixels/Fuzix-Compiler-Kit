;
;	 Left shift TOS.L by D
;
;	TODO optimize 8/16/24 bits of shift with moves first
;
	.export __shll
	.code

__shll:	clra
	ldx ,s
	andb #31
	beq load_out
shlnext:
	lsl 5,s
	rol 4,s
	rol 3,s
	rol 2,s
	decb
	bne shlnext
load_out:
	ldy 2,s
	ldd 4,s
	leas 6,s
	jmp ,x
