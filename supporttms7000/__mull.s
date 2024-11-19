	.export __mullu
	.export __mull

	.code

;
;	 TOS x ac
;
__mullu:
__mull:
	; r2-r5 x r10-r13
	; result into r6-r9 (so save first)
	call @__pop10
	call @__pop12

	push r6
	push r7
	push r8
	push r9

	; 32 x 8 low slice
	mpy r5,r13
	movd b,r9		; Save low result

	mpy r5,r12
	add b,r8
	mov a,r7

	mpy r5,r11
	add b,r7
	mov a,r6

	mpy r5,r10
	add b,r6

	; Second slice

	mpy r4,r13
	add b,r8
	add a,r7

	mpy r4,r12
	add b,r7
	add a,r6

	mpy r4,r11
	add b,r6

	; Third slice

	mpy r3,r13
	add b,r7
	add a,r6

	mpy r3,r12
	add b,r6

	; And done (any digits beyond are lost)

	movd r9,r5
	movd r7,r3
	pop r9
	pop r8
	pop r7
	pop r6
	rets