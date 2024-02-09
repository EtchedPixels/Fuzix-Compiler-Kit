;
;	16 x 16 divide core
;
	.export __div16x16
	.export __divu
	.export __remu
	.export __divequ
	.export __remequ

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

__divequ:
	pop r12		; Get arg off stack
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	lde r0,@rr14
	incw rr14
	lde r1,@rr14
	push r15
	push r14
	call __div16x16	; do the division
	pop r14
	pop r15
	ld r2,r12	; we want the quotient
	ld r3,r13
	lde @rr14,r3
	decw rr14
	lde @rr14,r2
	ret

__divequc:
	pop r12		; Get arg off stack
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	clr r0
	lde r1,@rr14
	clr r2
	push r15
	push r14
	call __div16x16	; do the division
	pop r14
	pop r15
	clr r2
	ld r3,r13
	lde @rr14,r3
	ret

__remequ:
	pop r12		; Get arg off stack
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	lde r0,@rr14
	incw rr14
	lde r1,@rr14
	push r15
	push r14
	call __div16x16	; do the division
	pop r14
	pop r15
	lde @rr14,r3
	decw rr14
	lde @rr14,r2
	ret

__remequc:
	pop r12		; Get arg off stack
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	clr r0
	lde r1,@rr14
	clr r2
	push r15
	push r14
	call __div16x16	; do the division
	pop r14
	pop r15
	clr r2
	lde @rr14,r3
	ret

__divs:
	pop r12
	pop r13
	pop r0
	pop r1
	push r13
	push r12
__dodivs:
	clr r12			; sign info
	tcm r0,#0x80		; negative ?
	jr z, uns1
	inc r12
	com r0
	com r1
	incw r0			; negate
uns1:	tcm r2,#0x80
	jr z, uns2
	inc r12
	com r2
	com r3
	incw r2
uns2:
	push r12		; remember sign info
	; Now we can do the divide unsigned
	call __div16x16
	pop r12
	rr r12
	jr nc, sign_ok
	com r0
	com r1
	incw r0
	ld r2,r0
	ld r3,r1
	ret

__rems:
	pop r12
	pop r13
	pop r0
	pop r1
	push r13
	push r12
__dorems:
	clr r12			; sign info
	tcm r0,#0x80		; negative ?
	jr nz, uns1
	com r0
	com r1
	incw r0			; negate
uns1:	tcm r2,#0x80
	jr nz, uns2
	inc r12
	com r2
	com r3
	incw r2
uns2:
	push r12		; remember sign info
	; Now we can do the divide unsigned
	call __div16x16
	pop r12
	rr r12
	jr nc, sign_ok
	com r2
	com r3
	incw r2
	ret

__diveq:
	pop r12		; Get arg off stack
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	lde r0,@rr14
	incw rr14
	lde r1,@rr14
	push r15
	push r14
	call __dodivs	; do the division
	pop r14
	pop r15
	lde @rr14,r3
	decw rr14
	lde @rr14,r2
	ret

__diveqc:
	pop r12		; Get arg off stack
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	clr r0
	lde r1,@rr14
	clr r2
	push r15
	push r14
	call __dodivs	; do the division
	pop r14
	pop r15
	clr r2
	lde @rr14,r3
	ret

__remeq:
	pop r12		; Get arg off stack
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	lde r0,@rr14
	incw rr14
	lde r1,@rr14
	push r15
	push r14
	call __dorems	; do the division
	pop r14
	pop r15
	lde @rr14,r3
	decw rr14
	lde @rr14,r2
	ret

__remeqc:
	pop r12		; Get arg off stack
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	clr r0
	lde r1,@rr14
	clr r2
	push r15
	push r14
	call __dorems	; do the division
	pop r14
	pop r15
	clr r2
	lde @rr14,r3
	ret

