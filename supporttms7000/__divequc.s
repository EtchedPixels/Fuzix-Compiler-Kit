	.export __divequc
	.code

__divequc:
	call @__pop12	; Get arg off stack
	clr r2
	lda *r13
	mov a,r3
	clr r4
	push r13
	push r12
	call @__div16x16	; do the division
	pop r12
	pop r13
	clr r2
	mov r3,r5
	mov r3,a
	sta *r13
	rets

