;
;	++ on a local at offset r13
;
	.export __lplusplusl
	.export __lplusplusbl
	.export __lplusplus1l
	.code

;
;	r13 points to the last byte
;
__lplusplus1l:
	mov %1,r5
__lplusplusbl:
	clr r2
	clr r3
	clr r4
__lplusplusl:
	clr r12
	add r15,r13
	adc r14,r12
	; Pointer is now in r12/r13, sum to add in r2-r5
	lda *r13
	mov a,b
	add r5,a
	mov b,r5
	sta *r13
	decd r13
	lda *r13
	mov a,b
	adc r4,a
	mov b,r4
	sta *r13
	decd r13
	lda *r13
	mov a,b
	adc r3,a
	mov b,r3
	sta *r13
	decd r13
	lda *r13
	mov a,b
	adc r2,a
	mov b,r2
	sta *r13
	rets

