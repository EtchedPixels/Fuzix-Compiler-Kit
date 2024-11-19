	.export __div
	.export __rem
	.export __diveq
	.export __remeq
	.export __diveqc
	.export __remeqc

	.code

__div:
	call @__pop2		; first arg ino r2,r3 working in r4,r5
__dodivs:
	clr r10			; sign info
	or r2,r2		; negative ?
	jp uns1
	inc r10
	inv r2
	inv r3
	add %1,r3
	adc %0,r2		; negate
uns1:	or r4,r4
	jp uns2
	inc r10			; sign memory
	inv r4
	inv r5	
	add %1,r5		; invert
	adc %0,r4
uns2:
	push r10		; remember sign info
	; Now we can do the divide unsigned
	call @__div16x16
	pop r10
	rr r10
	jnc sign_ok
	inv r2
	inv r3
	add %1,r3
	adc %0,r2
sign_ok:
	movd r1,r3
	rets

__rem:
	call @__pop2
__dorems:
	clr r10			; sign info
	or r2,r2		; negative ?
	jp uns3
	inc r10
	inv r2
	inv r3
	add %1,r3
	adc %0,r2		; negate
uns3:	or r4,r4
	jp uns4
	inv r4
	inv r5
	add %1,r5
	adc %0,r4
uns4:
	push r10		; remember sign info
	; Now we can do the divide unsigned
	call @__div16x16
	movd r11,r5
	pop r10
	rr r10
	jnc sign_ok_r
	inv r4
	inv r5
	add %1,r5
	adc %0,r4
sign_ok_r:
	rets

__diveq:
	call @__pop12	; Get ptr arg off stack
	lda *r13
	mov a,r2
	add %1,r13
	adc %0,r12
	lda *r13
	mov a,r3
	push r12
	push r13
	call @__dodivs	; do the division
store:
	pop r13
	pop r12
	mov r5,a
	sta *r13
	decd r13
	mov r4,a
	sta *r13
	rets

__diveqc:
	call @__pop12
	clr r2
	lda *r13
	mov a,r3
	clr r4
	push r12
	push r13
	call @__dodivs	; do the division
storec:
	pop r13
	pop r12
	clr r4
	mov r5,a
	sta *r13
	rets

__remeq:
	call @__pop12
	lda *r13
	mov a,r2
	add %1,r13
	adc %1,r12
	lda *r13
	mov a,r3
	push r12
	push r13
	call @__dorems	; do the division
	jmp store

__remeqc:
	call @__pop12
	clr r2
	lda *r13
	mov a,r3
	clr r4
	push r12
	push r13
	call @__dorems	; do the division
	jmp storec

