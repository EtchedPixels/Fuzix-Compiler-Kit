;
;	Same helper but with load/store from pointer
;
	.export __muleq
	.export __mulequ   	

	.code

__muleq:
__mulequ:
	; stack holds ptr instead in this case
	call @__pop10

	; Get values
	lda *r11
	mov a,r4
	add %1,r11
	adc %0,r11
	lda *r11
	mov a,r5

	; save ptr
	push r11
	push r10

	; r0,r1 x r2,r3
	call @__domul

	; result in r4,r5, r10/r11/12 trashed
	pop r10
	pop r11
	mov r5,a
	sta *r11
	decd r11
	mov r4,a
	sta *r11
	rets


