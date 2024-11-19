	.export __muleqc
	.export __mulequc   	

	.code

__muleqc:
__mulequc:
	; stack holds ptr instead in this case
	call @__pop10

	; Get values
	clr r2
	lda *r11
	mov a,r3

	; save ptr
	push r11
	push r10

	; r0,r1 x r2,r3
	call @__domul

	; result in r2,r3, r12/r13/14 trashed
	pop r10
	pop r11
	mov r5,a
	sta *r11
	clr r4
	rets
