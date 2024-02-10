;
;	16 x 16 divide core
;
	.export __div16x16
	.export __divu
	.export __remu

	.code


; Divide r0-r1 by r2-r3 
__div16x16:
	clr r12					; work D
	clr r13
	ld r14,#16				; tmp
divl:
	add r1,r1	; shift dividend left	; X
	adc r0,r0
	rlc r13		; rotate into work	; work D
	rlc r12
	; Is work bigger than divisor
	cp r12,r2				; D v tmp1
	jr nz, divl2
	cp r13,r3
divl2:	jr c, skipadd				; 
	sub r13,r3				; - divisor
	sbc r12,r2
	inc r1		; set low bit of r3 (we shifted it so it is 0 atm)
skipadd:
	djnz r14,divl
	;	At this point r12,r13 is the remainder
	;	r0,r1 is the quotient
	ret

__divu:
	pop r12		; Get arg off stack
	pop r13
	pop r0
	pop r1
	push r13
	push r12
	call __div16x16	; do the division
	ld r2,r0	; we want the quotient
	ld r3,r1
	ret

__remu:
	pop r12		; Get arg off stack
	pop r13
	pop r0					; dividend
	pop r1
	push r13
	push r12
	call __div16x16
	ld r2,r12
	ld r3,r13
	ret
