	.export __mulu
	.export __mul
	.export __domul

	.code

;
;	 TOS x ac
;
__mulu:
__mul:
	pop r14		; return address
	pop r15
	pop r0		; value
	pop r1
	push r15	; return address back
	push r14

	; Fall into the helper for r0/r1 x r2/r3

__domul:
	; r0,r1 x r2,r3  - r12-r15 free as scratch

	ld r12,r2	; save the value to multiply by
	ld r13,r3


	clr r2		; clear result
	clr r3

	ld r14,#16	; count

loop:
	; rotate sum left
	add r3,r3
	adc r2,r2
	; rotate r0,r1 left
	add r1,r1
	adc r0,r0

	; bit clear - skip add
	jr nc, noadd

	add r3,r13
	adc r2,r12
noadd:
	djnz r14,loop
	ret

