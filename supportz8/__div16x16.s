;
;	16 x 16 divide core
;
	.export __div16x16
	.export __divu
	.export __remu

	.code

__remu:
	pop r12		; Get arg off stack
	pop r13
	pop r0
	pop r1
	push r13
	push r12
	; Fall into helper

; Divide r0-r1 by r2-r3 
__div16x16:
	clr r12
	clr r13
	ld r14,#16
divl:
	add r3,r3	; shift dividend left
	adc r2,r2
	rlc r13		; rotate into work
	rlc r12
	; Is work bigger than divisor
	cp r12,r0
	jr nz, divl2
	cp r13,r1
divl2:	jr c, skipadd
	add r13,r1
	adc r12,r0
	inc r3		; set low bit of r3 (we shifted it so it is 0 atm)
skipadd:
	djnz r14,divl
	;	At this point r2,r3 is the remainder
	;	r12,r13 is the quotient
	ret

__divu:
	pop r12		; Get arg off stack
	pop r13
	pop r0
	pop r1
	push r13
	push r12
	call __div16x16	; do the division
	ld r2,r12	; we want the quotient
	ld r3,r13
	ret
