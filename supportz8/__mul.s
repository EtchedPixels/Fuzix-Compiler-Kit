;
;	Top of stack after return address is 
;
	.export __mulu
	.export __mul

	.code

__mulu:
__mul:
	pop r14		; return address
	pop r15
	pop r0		; value
	pop r1
	push r15	; return address back
	push r14

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
	jr z, noadd

	add r3,r13
	adc r2,r12

noadd:
	dec r14
	jr nz, loop

	ret

	.export __muleq
	.export __mulequ   	

__muleq:
__mulequ:
	; stack holds ptr instead in this case
	pop r14
	pop r15
	pop r12		; address
	pop r13
	push r15
	push r14

	; Get values
	lde r0, @rr12
	incw rr12
	lde r1, @rr12

	; save ptr
	push r13
	push r12

	; r0,r1 x r2,r3
	call __domul

	; result in r2,r3, r12/r13/14 trashed
	pop r12
	pop r13
	lde @rr12,r3
	decw rr12
	lde @rr12,r2
	ret

	.export __muleqc
	.export __mulequc   	

__muleqc:
__mulequc:
	; stack holds ptr instead in this case
	pop r14
	pop r15
	pop r12		; address
	pop r13
	push r15
	push r14

	; Get values
	clr r0
	lde r1, @rr12

	; save ptr
	push r13
	push r12

	; r0,r1 x r2,r3
	call __domul

	; result in r2,r3, r12/r13/14 trashed
	pop r12
	pop r13
	lde @rr12,r3
	clr r2
	ret
