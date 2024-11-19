
	.export __remequc
	.code
__remequc:
	call @__pop12
	clr r0
	lda *r13
	mov a,r5
	clr r4
	push r12
	push r13
	call @__div16x16	; do the division
	pop r13
	pop r12
	clr r4
	mov r5,a
	sta *r13
	rets

