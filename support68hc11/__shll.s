;
;	 Left shift TOS.L by D
;
;	TODO optimize 8/16/24 bits of shift with moves first
;
	.export __shll
	.code

__shll:	clra
	tsx
	andb #31
	beq load_out
shlnext:
	lsl 5,x
	rol 4,x
	rol 3,x
	rol 2,x
	decb
	bne shlnext
load_out:
	pulx
	; Above the return is the result 
	puly
	pula
	pulb
	jmp ,x
